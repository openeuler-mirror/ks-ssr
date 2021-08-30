/**
 * @file          /kiran-ssr-manager/src/tool/config/pam.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/tool/config/pam.h"
#include <fcntl.h>
#include "lib/base/file-lock.h"

namespace Kiran
{
namespace Config
{
PAM::PAM(const std::string &conf_path,
         const std::string &line_match_regex) : conf_path_(conf_path),
                                                line_match_pattern_(line_match_regex)
{
}

bool PAM::get(const std::string &key, const std::string &kv_split_pattern, std::string &value)
{
    std::string contents;
    RETURN_VAL_IF_FALSE(FileUtils::read_contents_with_lock(this->conf_path_, contents), false);

    auto lines = StrUtils::split_lines(contents);
    auto line_match_regex = Glib::Regex::create(this->line_match_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    std::string kv_pattern;
    if (kv_split_pattern.empty())
    {
        kv_pattern = key;
        value = "false";
    }
    else
    {
        kv_pattern = fmt::format("{0}{1}(\\S+)", key, kv_split_pattern);
    }
    auto kv_regex = Glib::Regex::create(kv_pattern);

    for (const auto &line : lines)
    {
        Glib::MatchInfo match_info;
        auto trim_line = StrUtils::trim(line);
        // 忽略空行和注释行
        CONTINUE_IF_TRUE(trim_line.empty() || trim_line[0] == '#');
        CONTINUE_IF_TRUE(!line_match_regex->match(line));

        if (kv_regex->match(line, match_info) &&
            match_info.matches())
        {
            value = kv_split_pattern.empty() ? "true" : match_info.fetch(1);
            return true;
        }
    }
    return true;
}

bool PAM::set(const std::string &key,
              const std::string &kv_split_pattern,
              const std::string &value,
              const std::string &kv_join_str)
{
    std::string new_contents;

    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::create_excusive_lock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING("Failed to lock file %s.", this->conf_path_.c_str());
        return false;
    }
    auto contents = Glib::file_get_contents(this->conf_path_);
    auto lines = StrUtils::split_lines(contents);
    auto line_match_regex = Glib::Regex::create(this->line_match_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    auto kv_pattern = kv_split_pattern.empty() ? fmt::format("({0})", key) : fmt::format("({0}{1})(\\S+)", key, kv_split_pattern);
    auto kv_regex = Glib::Regex::create(kv_pattern);

    bool replaced = false;

    for (const auto &line : lines)
    {
        auto trim_line = StrUtils::trim(line);

        if (replaced || trim_line.empty() || trim_line[0] == '#' || !line_match_regex->match(line))
        {
            new_contents.append(line);
            new_contents.push_back('\n');
            continue;
        }

        std::string new_line = line;
        if (kv_regex->match(line))
        {
            // 删除键值对
            if ((kv_split_pattern.empty() && StrUtils::tolower(value) == "false") ||
                value.empty())
            {
                new_line = kv_regex->replace(line, 0, Glib::ustring(), static_cast<Glib::RegexMatchFlags>(0));
            }
            // 修改键值对
            else if (!kv_split_pattern.empty() && !value.empty())
            {
                new_line = kv_regex->replace(line, 0, "\\g<1>" + value, static_cast<Glib::RegexMatchFlags>(0));
            }
        }
        else
        {
            // 添加键值对
            if (kv_split_pattern.empty() && StrUtils::tolower(value) == "true")
            {
                new_line += (this->is_whitespace_in_tail(line) ? "" : " ") + key;
            }
            else if (!kv_split_pattern.empty())
            {
                new_line += (this->is_whitespace_in_tail(line) ? "" : " ") + key + kv_join_str + value;
            }
        }

        KLOG_DEBUG("Replace line: %s with %s.", line.c_str(), new_line.c_str());

        new_contents.append(new_line);
        new_contents.push_back('\n');
        replaced = true;
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

bool PAM::del(const std::string &key, const std::string &kv_split_regex)
{
    return true;
}

bool PAM::is_whitespace_in_tail(const std::string &str)
{
    RETURN_VAL_IF_TRUE(str.empty(), false);
    return (isspace(str[str.length() - 1]) != 0);
}

}  // namespace Config

}  // namespace Kiran
