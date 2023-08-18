/**
 * @file          /kiran-ssr-manager/src/tool/config/plain.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/tool/config/plain.h"
#include <fcntl.h>
#include <set>
#include "lib/base/file-lock.h"

namespace Kiran
{
namespace Config
{
#define CONF_FILE_PERMISSION 0644

#define DEFAULT_SPLIT_REGEX "\\s+"
#define DEFAULT_JOIN_STRING "\t"

Plain::Plain(const std::string &conf_path,
             const std::string &delimiter_pattern,
             const std::string &join_string) : conf_path_(conf_path),
                                               delimiter_pattern_(delimiter_pattern),
                                               join_string_(join_string)
{
    if (this->delimiter_pattern_.empty())
    {
        this->delimiter_pattern_ = DEFAULT_SPLIT_REGEX;
    }

    if (this->join_string_.empty())
    {
        this->join_string_ = DEFAULT_JOIN_STRING;
    }
}

Plain::~Plain()
{
}

bool Plain::get(const std::string &key, std::string &value)
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

    for (const auto &line : lines)
    {
        auto trim_line = StrUtils::trim(line);
        // 忽略空行和注释行
        CONTINUE_IF_TRUE(trim_line.empty() || trim_line[0] == '#');
        std::vector<std::string> fields = split_field_regex->split(trim_line);
        // 只考虑两列的行
        CONTINUE_IF_TRUE(fields.size() != 2);
        KLOG_DEBUG("Read Line: key: %s, value: %s.", fields[0].c_str(), fields[1].c_str());

        if (fields[0] == key)
        {
            value = fields[1];
            return true;
        }
    }

    // 未找到不返回错误
    return true;
}

bool Plain::set(const std::string &key, const std::string &value)
{
    std::string new_contents;

    auto file_lock = FileLock::create_excusive_lock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING("Failed to lock file %s.", this->conf_path_.c_str());
        return false;
    }

    auto contents = Glib::file_get_contents(this->conf_path_);
    auto lines = StrUtils::split_lines(contents);

    auto split_field_regex = Glib::Regex::create(this->delimiter_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    auto second_field_regex = Glib::Regex::create(fmt::format("(\\s*\\S+{0})(\\S+)", this->delimiter_pattern_),
                                                  Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    bool replaced = false;

    for (const auto &line : lines)
    {
        auto trim_line = StrUtils::trim(line);
        std::vector<std::string> fields = split_field_regex->split(trim_line);

        if (trim_line.empty() || trim_line[0] == '#' || fields.size() != 2 || fields[0] != key)
        {
            new_contents.append(line);
            new_contents.push_back('\n');
            continue;
        }

        if (!value.empty())
        {
            auto replace_line = second_field_regex->replace(line, 0, "\\g<1>" + value, static_cast<Glib::RegexMatchFlags>(0));
            KLOG_DEBUG("Replace line: %s with %s.", line.c_str(), replace_line.c_str());
            new_contents.append(replace_line);
            new_contents.push_back('\n');
        }
        else
        {
            // 值为空则直接删除该行
            KLOG_DEBUG("Delete line: %s.", line.c_str());
        }
        replaced = true;
    }

    // 如果不存在该key且设置的值不为空，则在最后添加一行
    if (!replaced && !value.empty())
    {
        auto new_line = key + this->join_string_ + value + "\n";
        KLOG_DEBUG("New line: %s.", new_line.c_str());
        new_contents.append(new_line);
    }

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
    return true;
}

bool Plain::del(const std::string &key)
{
    KLOG_DEBUG("Key: %s.", key.c_str());
    return this->set(key, std::string());
}

}  // namespace Config
}  // namespace Kiran