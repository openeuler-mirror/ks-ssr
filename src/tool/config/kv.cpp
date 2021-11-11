/**
 * @file          /kiran-ssr-manager/src/tool/config/kv.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/tool/config/kv.h"
#include <fcntl.h>
#include <set>
#include "lib/base/file-lock.h"
#include "lib/base/file-utils.h"

namespace Kiran
{
namespace Config
{
#define DEFAULT_SPLIT_REGEX "\\s+"
#define DEFAULT_JOIN_STRING "\t"

KV::KV(const std::string &conf_path,
       const std::string &kv_split_pattern,
       const std::string &kv_join_str) : conf_path_(conf_path),
                                         kv_split_pattern_(kv_split_pattern),
                                         kv_join_str_(kv_join_str)
{
    if (this->kv_split_pattern_.empty())
    {
        this->kv_split_pattern_ = DEFAULT_SPLIT_REGEX;
    }

    if (this->kv_join_str_.empty())
    {
        this->kv_join_str_ = DEFAULT_JOIN_STRING;
    }
}

KV::~KV()
{
}

bool KV::get(const std::string &key, std::string &value)
{
    std::string contents;
    RETURN_VAL_IF_FALSE(FileUtils::read_contents_with_lock(this->conf_path_, contents), false);

    auto lines = StrUtils::split_lines(contents);
    auto split_field_regex = Glib::Regex::create(this->kv_split_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);

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

bool KV::set(const std::string &key, const std::string &value)
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

    auto split_field_regex = Glib::Regex::create(this->kv_split_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    auto second_field_regex = Glib::Regex::create(fmt::format("(\\s*\\S+{0})(\\S+)", this->kv_split_pattern_),
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
        auto new_line = key + this->kv_join_str_ + value + "\n";
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

bool KV::del(const std::string &key)
{
    KLOG_DEBUG("Key: %s.", key.c_str());
    return this->set(key, std::string());
}

}  // namespace Config
}  // namespace Kiran
