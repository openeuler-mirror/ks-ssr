/**
 * @file          /kiran-sse-manager/lib/config/config-pam.cpp  
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/config/config-pam.h"
#include <vector>
#include <set>

namespace Kiran
{
std::map<std::string, std::shared_ptr<ConfigPAM>> ConfigPAM::pams_ = std::map<std::string, std::shared_ptr<ConfigPAM>>();

ConfigPAM::ConfigPAM(const std::string &conf_path) : conf_path_(conf_path),
                                                         is_writing_(false)
{
    this->read_from_file();
    FileUtils::make_monitor_file(this->conf_path_, sigc::mem_fun(this, &ConfigPAM::on_config_file_changed));
}

ConfigPAM::~ConfigPAM()
{
}

void ConfigPAM::split_values()
{
    bool opera_flag = false;
    std::vector<std::string> new_values;
    auto split_field_regex = Glib::Regex::create("\\s+", Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    for(auto &iter : this->kvs_)
    {
        auto values = StrUtils::trim(iter.second);
        std::vector<std::string> fields = split_field_regex->split(values);
        std::string exchange_str;
        for( const auto &field : fields)
        {
            if(field == "=" || field == "<" || field == ">" || field == ">=" || field == "<=")
            {
                exchange_str.append(field);
                opera_flag = true;
                continue;
            }
            else
            {
                if(!opera_flag)
                {
                    exchange_str.clear();
                }
                else
                {
                    new_values.pop_back();
                    opera_flag = false;
                }
                exchange_str.append(field);
                new_values.push_back(exchange_str);
            }
        }
        this->kv_.emplace(iter.first, new_values);
    }
}

bool ConfigPAM::has_key(const std::string &key)
{
    auto iter = this->kvs_.find(key);
    return (iter != this->kvs_.end());
}

std::vector<std::string> ConfigPAM::get_all_value(const std::string &key)
{
    auto iter = this->kv_.find(key);
    RETURN_VAL_IF_TRUE(iter == this->kv_.end(), std::vector<std::string>());
    return iter->second;
}

std::string ConfigPAM::get_value(const std::string &key)
{
    auto iter = this->kvs_.find(key);
    RETURN_VAL_IF_TRUE(iter == this->kvs_.end(), std::string());
    return iter->second;
}

//XXX:可以添加在末尾添加未出现的pam参数，但是可能存在顺序问题，需要特殊处理。
bool ConfigPAM::set_value(const std::string &key, const std::string &value)
{
    KLOG_DEBUG("Key: %s, value: %s.", key.c_str(), value.c_str());

    auto value_regex = Glib::Regex::create("^([[:blank:]]|[^[:cntrl:]])*$");
    if (!value_regex->match(value))
    {
        KLOG_WARNING("The format of value '%s' is invalid.", value.c_str());
        return false;
    }

    value_regex = Glib::Regex::create(value);
    if(!value_regex->match(kvs_[key]))
    {
        kvs_[key].append(" " + value);
    }

    this->write_to_file_ready();
    return true;
}

bool ConfigPAM::set_value(const std::string &key, const std::string &value, const std::string &opera, const int num)
{
    KLOG_DEBUG("Key: %s, value: %s.", key.c_str(), value.c_str());

    auto value_regex = Glib::Regex::create("^([[:blank:]]|[^[:cntrl:]])*$");
    if (!value_regex->match(value))
    {
        KLOG_WARNING("The format of value '%s' is invalid.", value.c_str());
        return false;
    }

    auto regex_str = value + "[\\s]*[<=|>=|=|>|<]+[\\s]*[0-9]*";
    auto num_regex_str = value + "[\\s]*[<=|>=|=|>|<]+[\\s]*" + std::to_string(num);
    value_regex = Glib::Regex::create(regex_str, Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    auto replace_str = value + opera + std::to_string(num);

    if(value_regex->match(kvs_[key]))
    {
        auto num_regex = Glib::Regex::create(num_regex_str, Glib::RegexCompileFlags::REGEX_OPTIMIZE);
        if(!num_regex->match(kvs_[key]))
        {
            auto str = value_regex->replace(kvs_[key], 0, replace_str, static_cast<Glib::RegexMatchFlags>(0));
            kvs_[key] = str;
            KLOG_DEBUG("replace: %s for %s",  kvs_[key].c_str(), str.c_str());
        }
    }
    else
    {
        kvs_[key].append(" ");
        kvs_[key].append(replace_str);
    }

    this->write_to_file_ready();
    return true;
}

bool ConfigPAM::delete_key(const std::string &key)
{
    KLOG_DEBUG("Key: %s.", key.c_str());

    auto iter = this->kvs_.find(key);
    RETURN_VAL_IF_TRUE(iter == this->kvs_.end(), false);
    this->kvs_.erase(iter);
    this->write_to_file_ready();
    return true;
}

std::shared_ptr<ConfigPAM> ConfigPAM::create(const std::string &conf_path)
{
    auto iter = ConfigPAM::pams_.find(conf_path);
    if (iter != ConfigPAM::pams_.end())
    {
        return iter->second;
    }

    std::shared_ptr<ConfigPAM> pam(new ConfigPAM(conf_path));
    auto retval = ConfigPAM::pams_.emplace(conf_path, pam);
    if (!retval.second)
    {
        KLOG_WARNING("Failed to insert config %s.", conf_path.c_str());
        return nullptr;
    }
    return pam;
}

bool ConfigPAM::read_from_file()
{
    std::string contents;
    try
    {
        contents = Glib::file_get_contents(this->conf_path_);
    }
    catch (const Glib::FileError &e)
    {
        KLOG_WARNING("Failed to get file contents: %s.", this->conf_path_.c_str());
        return false;
    }

    auto lines = StrUtils::split_lines(contents);
    auto whitespace_regex = Glib::Regex::create("\\.so", Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    this->kvs_.clear();
    for (const auto &line : lines)
    {
        auto trim_line = StrUtils::trim(line);
        // 忽略空行和注释行
        CONTINUE_IF_TRUE(trim_line.empty() || trim_line[0] == '#');
        std::vector<std::string> fields = whitespace_regex->split(trim_line);
        // pam一行只存在一个.so，所以分割后应该剩下2行内容。
        CONTINUE_IF_TRUE(fields.size() != 2);

        fields[0].append(".so");
        auto iter = this->kvs_.emplace(fields[0], fields[1]);

        KLOG_DEBUG("Read Line: key: %s, value: %s.", fields[0].c_str(), fields[1].c_str());
        if (!iter.second)
        {
            KLOG_WARNING("The key %s is repeat, value: %s", fields[0].c_str(), fields[1].c_str());
        }
    }
    this->split_values();
    return true;
}

void ConfigPAM::write_to_file_ready()
{
    if (!this->write_file_idle_id_)
    {
        auto thread_context = Glib::wrap(g_main_context_get_thread_default());
        this->write_file_idle_id_ = thread_context->signal_idle().connect(sigc::mem_fun(this, &ConfigPAM::on_write_to_file_idle_cb));
    }
}

bool ConfigPAM::write_to_file()
{
    std::set<std::string> keys;
    for (auto &iter : this->kvs_)
    {
        keys.insert(iter.first);
    }

    std::string contents;
    std::string new_contents;

    if (Glib::file_test(this->conf_path_, Glib::FILE_TEST_EXISTS))
    {
        try
        {
            contents = Glib::file_get_contents(this->conf_path_);
        }
        catch (const Glib::Error &e)
        {
            KLOG_WARNING("Failed to get file contents: %s.", this->conf_path_.c_str());
            return false;
        }
    }

    auto lines = StrUtils::split_lines(contents);

    auto whitespace_regex = Glib::Regex::create("\\.so", Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    //auto second_field_regex = Glib::Regex::create("", Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    for (const auto &line : lines)
    {
        auto trim_line = StrUtils::trim(line);
        std::vector<std::string> fields = whitespace_regex->split(trim_line);

        if (trim_line.empty() || trim_line[0] == '#' || fields.size() != 2)
        {
            new_contents.append(line);
            new_contents.push_back('\n');
            continue;
        }

        fields[0].append(".so");
        // 如果key存在则使用缓存的值进行替换，否则直接删除该行
        auto iter = keys.find(fields[0]);
        if (iter != keys.end())
        {
            auto new_value = this->get_value(fields[0]);
            //auto replace_line = second_field_regex->replace(line, 0, "\\g<1>" + new_value, static_cast<Glib::RegexMatchFlags>(0));
            KLOG_DEBUG("Replace line: %s for %s.", (fields[0] + new_value).c_str(), line.c_str());
            auto replace_line = fields[0] + new_value;
            new_contents.append(replace_line);
            new_contents.push_back('\n');
            keys.erase(iter);
        }
        else
        {
            KLOG_DEBUG("Delete line: %s.", line.c_str());
        }
    }

    //TODO: 因pam配置有顺序关系，不能确定新增内容是否放在何处，暂时未实现该功能。
    // 剩下的未匹配的键值对为新添加的，直接放到最后
    // for (const auto &new_key : keys)
    // {
    //     auto new_value = this->get_value(new_key);
    //     auto new_line = new_key + "\t" + new_value + "\n";
    //     KLOG_DEBUG("New line: %s.", new_line.c_str());
    //     new_contents.append(new_line);
    // }

    {
        this->is_writing_ = true;
        SCOPE_EXIT({
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

bool ConfigPAM::on_write_to_file_idle_cb()
{
    this->write_to_file();
    return false;
}

void ConfigPAM::on_config_file_changed(const Glib::RefPtr<Gio::File> &file,
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
