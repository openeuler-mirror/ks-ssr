/**
 * @file          /ks-ssr-manager/src/tool/config/pam.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/tool/config/pam.h"
#include <fcntl.h>
#include "lib/base/file-lock.h"

namespace KS
{
namespace Config
{
PAM::PAM(const std::string &conf_path,
         const std::string &line_match_regex) : conf_path_(conf_path),
                                                line_match_pattern_(line_match_regex)
{
}

bool PAM::get_value(const std::string &key, const std::string &kv_split_pattern, std::string &value)
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
        kv_pattern = fmt::format("({0}[\\s]*{1}[\\s]*)(\\S+)", key, kv_split_pattern);
    }
    auto kv_regex = Glib::Regex::create(kv_pattern);

    auto split_field_regex = Glib::Regex::create(kv_split_pattern, Glib::RegexCompileFlags::REGEX_OPTIMIZE);

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
            if (!kv_split_pattern.empty())
            {
                std::vector<std::string> fields = split_field_regex->split(match_info.fetch(0));
                value = fields[1].c_str();
                KLOG_DEBUG("Read Line: key: %s, value: %s.", fields[0].c_str(), fields[1].c_str());
            }
            else
            {
                value = "true";
            }
            return true;
        }
    }
    return true;
}

bool PAM::set_value(const std::string &key,
                    const std::string &kv_split_pattern,
                    const std::string &value,
                    const std::string &kv_join_str)
{
    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::create_excusive_lock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING("Failed to lock file %s.", this->conf_path_.c_str());
        return false;
    }
    auto match_info = this->get_match_line();

    if (match_info.match_line.size() > 0 && !match_info.is_match_comment)
    {
        auto kv_pattern = kv_split_pattern.empty() ? fmt::format("({0})", key) : fmt::format("({0}[\\s]*{1}[\\s]*)(\\S+)", key, kv_split_pattern);
        auto kv_regex = Glib::Regex::create(kv_pattern);
        std::string replace_line = match_info.match_line;

        if (kv_regex->match(match_info.match_line))
        {
            // 修改键值对
            if (!kv_split_pattern.empty() && !value.empty())
            {
                replace_line = kv_regex->replace(match_info.match_line, 0, "\\g<1>" + value, static_cast<Glib::RegexMatchFlags>(0));
            }
        }
        else
        {
            // 添加键值对
            if (kv_split_pattern.empty())
            {
                replace_line += (this->is_whitespace_in_tail(match_info.match_line) ? "" : " ") + key;
            }
            else if (!kv_split_pattern.empty() && !value.empty())
            {
                replace_line += (this->is_whitespace_in_tail(match_info.match_line) ? "" : " ") + key + kv_join_str + value;
            }
            else
            {
                KLOG_WARNING("Unknown situation.");
            }
        }

        match_info.content.replace(match_info.match_pos, match_info.match_line.size(), replace_line);
        KLOG_DEBUG("Replace line: %s with %s.", match_info.match_line.c_str(), replace_line.c_str());
        return this->write_to_file(match_info.content);
    }
    return true;
}

bool PAM::del_value(const std::string &key, const std::string &kv_split_pattern)
{
    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::create_excusive_lock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING("Failed to lock file %s.", this->conf_path_.c_str());
        return false;
    }
    auto match_info = this->get_match_line();
    auto kv_pattern = kv_split_pattern.empty() ? fmt::format("({0})", key) : fmt::format("({0}[\\s]*{1}[\\s]*)(\\S+)", key, kv_split_pattern);
    auto kv_regex = Glib::Regex::create(kv_pattern);

    if (match_info.match_line.size() > 0 && !match_info.is_match_comment && kv_regex->match(match_info.match_line))
    {
        auto replace_line = kv_regex->replace(match_info.match_line, 0, Glib::ustring(), static_cast<Glib::RegexMatchFlags>(0));
        match_info.content.replace(match_info.match_pos, match_info.match_line.size(), replace_line);
        KLOG_DEBUG("Replace line: %s with %s.", match_info.match_line.c_str(), replace_line.c_str());
        return this->write_to_file(match_info.content);
    }
    return true;
}

bool PAM::add_line(const std::string &fallback_line, const std::string &next_line_match_regex)
{
    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::create_excusive_lock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING("Failed to lock file %s.", this->conf_path_.c_str());
        return false;
    }

    auto match_info = this->get_match_line();

    // 如果匹配到注释行，则取消注释
    if (match_info.match_line.size() > 0 && match_info.is_match_comment)
    {
        auto replace_line = match_info.match_line.substr(1);
        match_info.content.replace(match_info.match_pos, match_info.match_line.size(), replace_line);
        KLOG_DEBUG("Replace line: %s with %s.", match_info.match_line.c_str(), replace_line.c_str());
        this->write_to_file(match_info.content);
    }
    // 如果未匹配到行，则添加新行
    else if (match_info.match_line.size() == 0 && fallback_line.size() > 0)
    {
        // KLOG_DEBUG("New line: %s.", fallback_line.c_str());
        // match_info.content.append(fallback_line);
        // this->write_to_file(match_info.content);
        auto new_match_info = this->add_behind(fallback_line, next_line_match_regex);
        this->write_to_file(new_match_info.content);
    }
    return true;
}

bool PAM::del_line()
{
    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::create_excusive_lock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING("Failed to lock file %s.", this->conf_path_.c_str());
        return false;
    }

    auto match_info = this->get_match_line();

    if (match_info.match_line.size() > 0 && !match_info.is_match_comment)
    {
        match_info.content.replace(match_info.match_pos, match_info.match_line.size(), "#" + match_info.match_line);
        KLOG_DEBUG("Comment line: %s with #.", match_info.match_line.c_str());
        this->write_to_file(match_info.content);
    }
    return true;
}

bool PAM::get_line(std::string &line)
{
    auto file_lock = FileLock::create_share_lock(this->conf_path_, O_RDONLY, 0);
    if (!file_lock)
    {
        KLOG_DEBUG("Failed to create share lock for %s.", this->conf_path_.c_str());
        return false;
    }

    auto match_info = this->get_match_line();

    if (match_info.match_line.size() > 0 && !match_info.is_match_comment)
    {
        line = match_info.match_line;
    }
    return true;
}

PAM::MatchLineInfo PAM::get_match_line()
{
    MatchLineInfo retval;

    auto contents = Glib::file_get_contents(this->conf_path_);
    auto lines = StrUtils::split_lines(contents);
    auto line_match_regex = Glib::Regex::create(this->line_match_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    // 寻找匹配行，如果没有匹配的非注释行可用，则使用匹配的注释行（注释将被去掉）
    for (const auto &line : lines)
    {
        std::vector<std::string> fields;

        // 注释行判断需要包括前面的空白字符
        bool is_comment = StrUtils::startswith(line, "#");

        if (line_match_regex->match(line) &&
            (retval.match_line.size() == 0 || (int32_t(is_comment) < int32_t(retval.is_match_comment))))
        {
            retval.match_pos = retval.content.size();
            retval.match_line = line;
            retval.is_match_comment = is_comment;
        }

        retval.content.append(line);
        retval.content.push_back('\n');
    }

    KLOG_DEBUG("match line: %s is comment: %d.", retval.match_line.c_str(), retval.is_match_comment);
    return retval;
}

PAM::MatchLineInfo PAM::add_behind(const std::string &fallback_line, const std::string &next_line_match_regex)
{
    MatchLineInfo retval;

    auto contents = Glib::file_get_contents(this->conf_path_);
    auto lines = StrUtils::split_lines(contents);
    auto line_match_regex = Glib::Regex::create(next_line_match_regex, Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    // 寻找匹配行，如果没有匹配的非注释行可用，则使用匹配的注释行（注释将被去掉）
    for (const auto &line : lines)
    {
        std::vector<std::string> fields;

        if (line_match_regex->match(line) &&
            (retval.match_line.size() == 0))
        {
            retval.content.append(fallback_line);
            retval.content.push_back('\n');
            retval.match_pos = retval.content.size();
            retval.match_line = line;
        }

        retval.content.append(line);
        retval.content.push_back('\n');
    }

    KLOG_DEBUG("match line: %s is comment: %d.", retval.match_line.c_str(), retval.is_match_comment);
    return retval;
}

bool PAM::write_to_file(const std::string &content)
{
    try
    {
        KLOG_DEBUG("New contents: %s.", content.c_str());
        FileUtils::write_contents(this->conf_path_, content);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to write file contents: %s.", this->conf_path_.c_str());
        return false;
    }
    return true;
}

bool PAM::is_whitespace_in_tail(const std::string &str)
{
    RETURN_VAL_IF_TRUE(str.empty(), false);
    return (isspace(str[str.length() - 1]) != 0);
}

}  // namespace Config

}  // namespace KS
