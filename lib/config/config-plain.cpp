/**
 * @file          /kiran-ssr-manager/lib/config/config-plain.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/config/config-plain.h"
#include <fcntl.h>
#include <set>
#include "lib/base/file-lock.h"

namespace Kiran
{
#define CONF_FILE_PERMISSION 0644

std::map<std::string, std::shared_ptr<ConfigPlain>> ConfigPlain::plains_ = std::map<std::string, std::shared_ptr<ConfigPlain>>();

std::shared_ptr<ConfigPlain> ConfigPlain::create(const std::string &conf_path,
                                                 const std::string &delimiter_pattern,
                                                 const std::string &insert_delimiter)
{
    static std::mutex mutex;
    {
        std::unique_lock<std::mutex> lck(mutex);
        auto iter = ConfigPlain::plains_.find(conf_path);
        if (iter != ConfigPlain::plains_.end())
        {
            return iter->second;
        }

        std::shared_ptr<ConfigPlain> plain(new ConfigPlain(conf_path, delimiter_pattern, insert_delimiter));
        auto retval = ConfigPlain::plains_.emplace(conf_path, plain);
        if (!retval.second)
        {
            KLOG_WARNING("Failed to insert config %s.", conf_path.c_str());
            return nullptr;
        }
        return plain;
    }
}

ConfigPlain::ConfigPlain(const std::string &conf_path,
                         const std::string &delimiter_pattern,
                         const std::string &insert_delimiter) : conf_path_(conf_path),
                                                                delimiter_pattern_(delimiter_pattern),
                                                                insert_delimiter_(insert_delimiter),
                                                                is_writing_(false)
{
    this->read_from_file();
    FileUtils::make_monitor_file(this->conf_path_, sigc::mem_fun(this, &ConfigPlain::on_config_file_changed));
}

ConfigPlain::~ConfigPlain()
{
}

bool ConfigPlain::has_key(const std::string &key)
{
    std::unique_lock<std::mutex> lck(this->mutex_);
    auto iter = this->kvs_.find(key);
    return (iter != this->kvs_.end());
}

std::string ConfigPlain::get_value(const std::string &key)
{
    std::unique_lock<std::mutex> lck(this->mutex_);
    auto iter = this->kvs_.find(key);
    RETURN_VAL_IF_TRUE(iter == this->kvs_.end(), std::string());
    return iter->second;
}

bool ConfigPlain::set_value(const std::string &key, const std::string &value)
{
    KLOG_DEBUG("Key: %s, value: %s.", key.c_str(), value.c_str());

    auto key_regex = Glib::Regex::create("^[A-Za-z_][A-Za-z0-9_.]*$");
    auto value_regex = Glib::Regex::create("^([[:blank:]]|[^[:cntrl:]])*$");
    if (!key_regex->match(key))
    {
        KLOG_WARNING("The format of key '%s' is invalid.", key.c_str());
        return false;
    }

    if (!value_regex->match(value))
    {
        KLOG_WARNING("The format of value '%s' is invalid.", value.c_str());
        return false;
    }

    {
        std::unique_lock<std::mutex> lck(this->mutex_);
        this->kvs_[key] = value;
    }
    this->write_to_file_ready();
    return true;
}

bool ConfigPlain::delete_key(const std::string &key)
{
    KLOG_DEBUG("Key: %s.", key.c_str());

    {
        std::unique_lock<std::mutex> lck(this->mutex_);
        auto iter = this->kvs_.find(key);
        RETURN_VAL_IF_TRUE(iter == this->kvs_.end(), false);
        this->kvs_.erase(iter);
    }
    this->write_to_file_ready();
    return true;
}

bool ConfigPlain::is_bool(const std::string &key)
{
    auto value_str = this->get_value(key);
    auto lower_value = StrUtils::tolower(value_str);

    RETURN_VAL_IF_TRUE(value_str.empty(), false);
    RETURN_VAL_IF_TRUE(lower_value == "true" || lower_value == "false", true);

    if (value_str.length() == 1 &&
        (value_str[0] == '1' || value_str[0] == '0'))
    {
        return true;
    }
    return false;
}

bool ConfigPlain::get_bool(const std::string &key)
{
    auto value_str = this->get_value(key);
    RETURN_VAL_IF_TRUE(value_str.empty(), false);
    RETURN_VAL_IF_TRUE(StrUtils::tolower(value_str) == "true", true);
    RETURN_VAL_IF_TRUE(StrUtils::tolower(value_str) == "false", false);

    if (value_str.length() == 1 && value_str[0] == '1')
    {
        return true;
    }
    return false;
}

bool ConfigPlain::is_integer(const std::string &key)
{
    auto value_str = this->get_value(key);
    RETURN_VAL_IF_TRUE(value_str.empty(), false);

    char *endptr = NULL;
    std::strtoll(value_str.c_str(), &endptr, 0);
    RETURN_VAL_IF_TRUE(endptr == value_str.data() + value_str.length(), true);
    return false;
}

int64_t ConfigPlain::get_integer(const std::string &key)
{
    auto value_str = this->get_value(key);
    RETURN_VAL_IF_TRUE(value_str.empty(), 0);
    auto value = std::strtoll(value_str.c_str(), nullptr, 0);
    return value;
}

bool ConfigPlain::is_double(const std::string &key)
{
    auto value_str = this->get_value(key);
    RETURN_VAL_IF_TRUE(value_str.empty(), false);

    char *endptr = NULL;
    std::strtod(value_str.c_str(), &endptr);
    RETURN_VAL_IF_TRUE(endptr == value_str.data() + value_str.length(), true);
    return false;
}

double ConfigPlain::get_double(const std::string &key)
{
    auto value_str = this->get_value(key);
    RETURN_VAL_IF_TRUE(value_str.empty(), 0);
    auto value = std::strtod(value_str.c_str(), nullptr);
    return value;
}

bool ConfigPlain::read_from_file()
{
    std::string contents;
    {
        auto file_lock = FileLock::create_share_lock(this->conf_path_, O_RDONLY, 0);
        if (!file_lock)
        {
            KLOG_DEBUG("Failed to create share lock for %s.", this->conf_path_.c_str());
            return false;
        }

        try
        {
            contents = Glib::file_get_contents(this->conf_path_);
        }
        catch (const Glib::FileError &e)
        {
            KLOG_WARNING("Failed to get file contents: %s.", this->conf_path_.c_str());
            return false;
        }
    }

    auto lines = StrUtils::split_lines(contents);
    auto split_field_regex = Glib::Regex::create(this->delimiter_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    {
        std::unique_lock<std::mutex> lck(this->mutex_);
        this->kvs_.clear();
        for (const auto &line : lines)
        {
            auto trim_line = StrUtils::trim(line);
            // 忽略空行和注释行
            CONTINUE_IF_TRUE(trim_line.empty() || trim_line[0] == '#');
            std::vector<std::string> fields = split_field_regex->split(trim_line);
            // 只考虑两列的行
            CONTINUE_IF_TRUE(fields.size() != 2);
            auto iter = this->kvs_.emplace(fields[0], fields[1]);
            KLOG_DEBUG("Read Line: key: %s, value: %s.", fields[0].c_str(), fields[1].c_str());
            if (!iter.second)
            {
                KLOG_WARNING("The key %s is repeat, value: %s", fields[0].c_str(), fields[1].c_str());
            }
        }
    }
    return true;
}

void ConfigPlain::write_to_file_ready()
{
    // 空闲时在主线程中保存数据
    std::unique_lock<std::mutex> lck(this->mutex_);
    if (!this->write_file_idle_id_)
    {
        auto context = Glib::MainContext::get_default();
        this->write_file_idle_id_ = context->signal_idle().connect(sigc::mem_fun(this, &ConfigPlain::on_write_to_file_idle_cb));
    }
}

bool ConfigPlain::write_to_file()
{
    std::set<std::string> keys;
    {
        std::unique_lock<std::mutex> lck(this->mutex_);
        for (auto &iter : this->kvs_)
        {
            keys.insert(iter.first);
        }
    }

    std::string contents;
    std::string new_contents;

    if (Glib::file_test(this->conf_path_, Glib::FILE_TEST_EXISTS))
    {
        try
        {
            auto file_lock = FileLock::create_share_lock(this->conf_path_, O_RDONLY, 0);
            if (file_lock)
            {
                contents = Glib::file_get_contents(this->conf_path_);
            }
        }
        catch (const Glib::Error &e)
        {
            KLOG_WARNING("Failed to get file contents: %s.", this->conf_path_.c_str());
            return false;
        }
    }

    auto lines = StrUtils::split_lines(contents);

    auto split_field_regex = Glib::Regex::create(this->delimiter_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    auto second_field_regex = Glib::Regex::create(fmt::format("(\\s*\\S+{0})(\\S+)", this->delimiter_pattern_),
                                                  Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    for (const auto &line : lines)
    {
        auto trim_line = StrUtils::trim(line);
        std::vector<std::string> fields = split_field_regex->split(trim_line);

        if (trim_line.empty() || trim_line[0] == '#' || fields.size() != 2)
        {
            new_contents.append(line);
            new_contents.push_back('\n');
            continue;
        }

        // 如果key存在则使用缓存的值进行替换，否则直接删除该行
        auto iter = keys.find(fields[0]);
        if (iter != keys.end())
        {
            auto new_value = this->get_value(fields[0]);
            auto replace_line = second_field_regex->replace(line, 0, "\\g<1>" + new_value, static_cast<Glib::RegexMatchFlags>(0));
            KLOG_DEBUG("Replace line: %s for %s.", replace_line.c_str(), line.c_str());
            new_contents.append(replace_line);
            new_contents.push_back('\n');
            keys.erase(iter);
        }
        else
        {
            KLOG_DEBUG("Delete line: %s.", line.c_str());
        }
    }

    // 剩下的未匹配的键值对为新添加的，直接放到最后
    for (const auto &new_key : keys)
    {
        auto new_value = this->get_value(new_key);
        auto new_line = new_key + this->insert_delimiter_ + new_value + "\n";
        KLOG_DEBUG("New line: %s.", new_line.c_str());
        new_contents.append(new_line);
    }

    auto file_lock = FileLock::create_excusive_lock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_DEBUG("Failed to create share lock");
    }
    else
    {
        this->is_writing_ = true;
        SSR_SCOPE_EXIT({
            this->is_writing_ = false;
        });

        try
        {
            KLOG_DEBUG("New contents: %s.", new_contents.c_str());
            FileUtils::write_contents(this->conf_path_, new_contents);
        }
        catch (const Glib::Error &e)
        {
            KLOG_WARNING("Failed to write file contents: %s.", this->conf_path_.c_str());
            return false;
        }
    }

    return true;
}

bool ConfigPlain::on_write_to_file_idle_cb()
{
    this->write_to_file();
    return false;
}

void ConfigPlain::on_config_file_changed(const Glib::RefPtr<Gio::File> &file,
                                         const Glib::RefPtr<Gio::File> &other_file,
                                         Gio::FileMonitorEvent event_type)
{
    KLOG_DEBUG("event type: %d, is writing: %d.", event_type, this->is_writing_);

    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED &&
                   event_type != Gio::FILE_MONITOR_EVENT_CREATED &&
                   event_type != Gio::FILE_MONITOR_EVENT_DELETED);
    RETURN_IF_TRUE(this->is_writing_);

    this->read_from_file();
}
}  // namespace Kiran
