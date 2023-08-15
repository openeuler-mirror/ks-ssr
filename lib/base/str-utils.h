/**
 * @file          /kiran-sse-manager/lib/base/str-utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include <fmt/format.h>
#include <giomm.h>
#include <json/json.h>

namespace fmt
{
template <>
struct formatter<Glib::ustring>
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext &ctx) const
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    format_context::iterator format(const Glib::ustring &str, FormatContext &ctx)
    {
        return format_to(ctx.out(), "{0}", str.raw());
    }
};
}  // namespace fmt

namespace Kiran
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
}  // namespace Kiran