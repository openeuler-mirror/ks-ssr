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
#define DEFAULT_COMMENT_STRING "#"

KV::KV(const std::string &conf_path,
       const std::string &kv_split_pattern,
       const std::string &kv_join_str,
       const std::string &comment) : conf_path_(conf_path),
                                     kv_split_pattern_(kv_split_pattern),
                                     kv_join_str_(kv_join_str),
                                     comment_(comment)
{
    if (this->kv_split_pattern_.empty())
    {
        this->kv_split_pattern_ = DEFAULT_SPLIT_REGEX;
    }

    if (this->kv_join_str_.empty())
    {
        this->kv_join_str_ = DEFAULT_JOIN_STRING;
    }

    if (this->comment_.empty())
    {
        this->comment_ = DEFAULT_COMMENT_STRING;
    }
}

KV::~KV()
{
}

bool KV::get(const std::string &key, std::string &value)
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

    bool is_matched = false;
    int32_t match_pos = 0;
    bool is_match_comment = false;
    std::string match_line;

    // 寻找匹配key的行，如果没有匹配的非注释行可用，则使用匹配的注释行（注释将被去掉）

    for (const auto &line : lines)
    {
        std::vector<std::string> fields;

        // 注释行判断需要包括前面的空白字符
        bool is_comment = StrUtils::startswith(line, this->comment_);

        if (is_comment)
        {
            auto trim_line = StrUtils::trim(line.substr(this->comment_.size()));
            fields = split_field_regex->split(trim_line);
        }
        else
        {
            auto trim_line = StrUtils::trim(line);
            fields = split_field_regex->split(trim_line);
        }

        if (!is_matched&&
	    fields.size() == 2 &&
            fields[0] == key &&
            (match_line.size() == 0 || (int32_t(is_comment) <= int32_t(is_match_comment))))
        {
            match_pos = new_contents.size();
            match_line = line;
            is_match_comment = is_comment;
	    is_matched = true;
        }

        new_contents.append(line);
        new_contents.push_back('\n');
    }

    KLOG_DEBUG("match line: %s is comment: %d.", match_line.c_str(), is_match_comment);

    if (match_line.size() > 0)
    {
        if (value.empty())
        {
            // 如果不是注释行，则进行注释，否则不处理
            if (!is_match_comment)
            {
                new_contents.replace(match_pos, match_line.size(), this->comment_ + match_line);
                KLOG_DEBUG("Comment line: %s with %s.", match_line.c_str(), this->comment_.c_str());
            }
        }
        else
        {
            // 如果存在注释则去掉，然后对value进行修改
            auto uncomment_line = is_match_comment ? match_line.substr(this->comment_.size()) : match_line;
            auto replace_line = second_field_regex->replace(uncomment_line, 0, "\\g<1>" + value, static_cast<Glib::RegexMatchFlags>(0));
            new_contents.replace(match_pos, match_line.size(), replace_line);
            KLOG_DEBUG("Replace line: %s with %s.", match_line.c_str(), replace_line.c_str());
        }
    }
    else
    {
        // 如果不存在匹配行且value不为空，则在最后添加一行
        if (!value.empty())
        {
            auto new_line = key + this->kv_join_str_ + value + "\n";
            KLOG_DEBUG("New line: %s.", new_line.c_str());
            new_contents.append(new_line);
        }
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

bool KV::set_all(const std::string &key, const std::string &value)
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

    int32_t match_pos = 0;
    bool is_match_comment = false;
    bool is_matching = false;

    // 寻找匹配key的行，如果没有匹配的非注释行可用，则使用匹配的注释行（注释将被去掉）
    for (const auto &line : lines)
    {
        std::string match_line = "";
        std::vector<std::string> fields;

        // 注释行判断需要包括前面的空白字符
        bool is_comment = StrUtils::startswith(line, this->comment_);

        if (is_comment)
        {
            auto trim_line = StrUtils::trim(line.substr(this->comment_.size()));
            fields = split_field_regex->split(trim_line);
        }
        else
        {
            auto trim_line = StrUtils::trim(line);
            fields = split_field_regex->split(trim_line);
        }

        if (fields.size() == 2 &&
            fields[0] == key &&
            (match_line.size() == 0 || (int32_t(is_comment) <= int32_t(is_match_comment))))
        {
            match_pos = new_contents.size();
            match_line = line;
            is_match_comment = is_comment;
        }

        new_contents.append(line);
        new_contents.push_back('\n');

        if (match_line.size() > 0)
        {
            // 匹配成功标志位，如果匹配到，就不添加新的参数
            is_matching = true;
            if (value.empty())
            {
                // 如果不是注释行，则进行注释，否则不处理
                if (!is_match_comment)
                {
                    new_contents.replace(match_pos, match_line.size(), this->comment_ + match_line);
                    KLOG_DEBUG("Comment line: %s with %s.", match_line.c_str(), this->comment_.c_str());
                }
            }
            else
            {
                // 如果存在注释则去掉，然后对value进行修改
                auto uncomment_line = is_match_comment ? match_line.substr(this->comment_.size()) : match_line;
                auto replace_line = second_field_regex->replace(uncomment_line, 0, "\\g<1>" + value, static_cast<Glib::RegexMatchFlags>(0));
                new_contents.replace(match_pos, match_line.size(), replace_line);
                KLOG_DEBUG("contents: %s.", new_contents.c_str());
                KLOG_DEBUG("Replace line: %s with %s.", match_line.c_str(), replace_line.c_str());
            }
        }
    }

    if(!is_matching)
    {
        // 如果不存在匹配行且value不为空，则在最后添加一行
        if (!value.empty())
        {
            auto new_line = key + this->kv_join_str_ + value + "\n";
            KLOG_DEBUG("New line: %s.", new_line.c_str());
            new_contents.append(new_line);
        }
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
    // 这里偷了一个懒，合理做法是需要单独实现删除逻辑，set函数不应该执行删除操作
    KLOG_DEBUG("Key: %s.", key.c_str());
    return this->set(key, std::string());
}

}  // namespace Config
}  // namespace Kiran
