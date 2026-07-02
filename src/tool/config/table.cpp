/**
 * @file          /kiran-ssr-manager/src/tool/config/table.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/tool/config/table.h"
#include <fcntl.h>
#include <set>
#include "lib/base/file-lock.h"
#include "lib/base/file-utils.h"

namespace Kiran
{
namespace Config
{
#define DEFAULT_JOIN_STRING "\t"

Table::Table(const std::string &conf_path,
             const std::string &split_pattern,
             const std::string &join_str) : conf_path_(conf_path),
                                            split_pattern_(split_pattern),
                                            join_str_(join_str)
{
    if (this->join_str_.empty())
    {
        this->join_str_ = DEFAULT_JOIN_STRING;
    }
}

Table::~Table()
{
}

bool Table::get(std::function<bool(std::vector<std::string>)> pred, std::string &value)
{
    // 文件如果不存在则返回空字符串
    if (!Glib::file_test(this->conf_path_, Glib::FILE_TEST_EXISTS))
    {
        value = std::string();
        return true;
    }

    std::string contents;
    RETURN_VAL_IF_FALSE(FileUtils::read_contents_with_lock(this->conf_path_, contents), false);

    auto lines = StrUtils::split_lines(contents);
    auto split_field_regex = Glib::Regex::create(this->split_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    for (const auto &line : lines)
    {
        std::vector<std::string> fields;
        auto trim_line = StrUtils::trim(line);
        // 忽略空行和注释行
        CONTINUE_IF_TRUE(trim_line.empty() || trim_line[0] == '#');
        // 如果分割的正则未设置，默认不分割，一整行表示一列
        if (this->split_pattern_.empty())
        {
            fields.push_back(trim_line);
        }
        else
        {
            fields = split_field_regex->split(trim_line);
        }

        if (pred(fields))
        {
            value = line;
            return true;
        }
    }
    return true;
}

bool Table::set(const std::string &newline, std::function<bool(std::vector<std::string>)> pred)
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

    auto split_field_regex = Glib::Regex::create(this->split_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    auto second_field_regex = Glib::Regex::create(fmt::format("(\\s*\\S+{0})(\\S+)", this->split_pattern_),
                                                  Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    bool has_matched = false;

    for (const auto &line : lines)
    {
        auto trim_line = StrUtils::trim(line);
        std::vector<std::string> fields;

        // 如果分割的正则未设置，默认不分割，一整行表示一列
        if (this->split_pattern_.empty())
        {
            fields.push_back(trim_line);
        }
        else
        {
            fields = split_field_regex->split(trim_line);
        }

        if (trim_line.empty() || trim_line[0] == '#' || !pred(fields) || has_matched)
        {
            new_contents.append(line);
            new_contents.push_back('\n');
            continue;
        }

        if (!newline.empty())
        {
            KLOG_DEBUG("Replace line: %s with %s.", line.c_str(), newline.c_str());
            new_contents.append(newline);
            new_contents.push_back('\n');
            has_matched = true;
        }
        else
        {
            // 值为空则直接删除该行
            KLOG_DEBUG("Delete line: %s.", line.c_str());
        }
    }

    // 如果不存在该行且设置的值不为空，则在最后添加一行
    if (!has_matched && !newline.empty())
    {
        KLOG_DEBUG("New line: %s.", newline.c_str());
        new_contents.append(newline);
        new_contents.push_back('\n');
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

bool Table::del(std::function<bool(std::vector<std::string>)> pred)
{
    return this->set(std::string(), pred);
}

}  // namespace Config
}  // namespace Kiran
