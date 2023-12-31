/**
 * @file          /ks-ssr-manager/lib/base/str-utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include <fmt/format.h>
#include <giomm.h>
#include <json/json.h>

namespace KS
{
class StrUtils
{
public:
    StrUtils(){};
    virtual ~StrUtils(){};

    static std::vector<std::string> split_lines(const std::string &s);
    static std::string tolower(const std::string &str);
    static std::string toupper(const std::string &str);
    static std::vector<std::string> split_with_char(const std::string &s, char delimiter, bool is_merge_delimiter = false);

    // 去掉字符串前面的空白字符
    static std::string ltrim(const std::string &s);
    // 去掉字符串后面的空白字符
    static std::string rtrim(const std::string &s);
    // 去掉字符串前后的空白字符
    static std::string trim(const std::string &s);

    static std::string json2str(const Json::Value &json);
    static Json::Value str2json(const std::string &str);

    // 判断str是否以prefix字符串开头
    static bool startswith(const std::string &str, const std::string &prefix);

    // 字符串列表交集
    static std::vector<std::string> intersect(const std::vector<std::string> &a1, const std::vector<std::string> &a2);

    template <class T>
    static std::string join(const std::vector<T> &vec, const std::string &join_chars);
};

template <class T>
std::string StrUtils::join(const std::vector<T> &vec, const std::string &join_chars)
{
    std::string str;
    for (size_t i = 0; i < vec.size(); ++i)
    {
        str += fmt::format("{0}", vec[i]);
        if (i + 1 < vec.size())
        {
            str += fmt::format("{0}", join_chars);
        }
    }
    return str;
}
}  // namespace KS