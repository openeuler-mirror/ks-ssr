/**
 * @file          /kiran-sse-manager/lib/base/str-utils.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/str-utils.h"
#include <algorithm>
#include "gtk3-log-i.h"
#include "lib/base/def.h"

namespace Kiran
{
std::vector<std::string> StrUtils::split_lines(const std::string &s)
{
    std::vector<std::string> ret;
    size_t i = 0;
    size_t line_start = 0;
    while (i < s.length())
    {
        if (s[i] == '\n')
        {
            ret.push_back(s.substr(line_start, i - line_start));
            i++;
            line_start = i;
        }
        else if (s[i] == '\r')
        {
            if ((i + 1 < s.length() && s[i + 1] != '\n') ||
                i + 1 >= s.length())
            {
                ret.push_back(s.substr(line_start, i - line_start));
                i++;
                line_start = i;
            }
            else  // if (i + 1 < s.length() && s[i + 1] == '\n')
            {
                ret.push_back(s.substr(line_start, i - line_start));
                i += 2;
                line_start = i;
            }
        }
        else
        {
            i++;
        }
    }
    ret.push_back(s.substr(line_start, s.length() - line_start));
    return ret;
}

std::string StrUtils::tolower(const std::string &str)
{
    std::string new_str = str;
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), ::tolower);
    return new_str;
}

std::string StrUtils::toupper(const std::string &str)
{
    std::string new_str = str;
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), ::toupper);
    return new_str;
}

std::vector<std::string> StrUtils::split_with_char(const std::string &s, char delimiter, bool is_merge_delimiter)
{
    std::vector<std::string> v;
    size_t start = 0;
    size_t i = 0;
    while (i < s.length())
    {
        if (delimiter == s[i])
        {
            if (i > start || !is_merge_delimiter)
            {
                v.push_back(s.substr(start, i - start));
            }
            i++;
            start = i;
        }
        else
        {
            i++;
        }
    }
    v.push_back(s.substr(start, s.length() - start));
    return v;
}

std::string StrUtils::ltrim(const std::string &s)
{
    auto iter = std::find_if(s.begin(), s.end(), [](char c) -> bool { return (std::isspace(c) == 0); });
    return std::string(iter, s.end());
}

std::string StrUtils::rtrim(const std::string &s)
{
    auto iter = std::find_if(s.rbegin(), s.rend(), [](char c) -> bool { return (std::isspace(c) == 0); });
    return std::string(s.begin(), iter.base());
}

std::string StrUtils::trim(const std::string &s)
{
    return StrUtils::ltrim(StrUtils::rtrim(s));
}

std::string StrUtils::json2str(const Json::Value &json)
{
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "";
    return Json::writeString(wbuilder, json);
}

Json::Value StrUtils::str2json(const std::string &str)
{
    KLOG_DEBUG("json str: %s.", str.c_str());
    Json::Value result;
    Json::CharReaderBuilder rbuilder;
    std::unique_ptr<Json::CharReader> reader(rbuilder.newCharReader());
    std::string error;

    RETURN_VAL_IF_TRUE(str.empty(), Json::Value());

    if (!reader->parse(str.c_str(), str.c_str() + str.length(), &result, &error))
    {
        KLOG_WARNING("%s", error.c_str());
        return Json::Value();
    }
    return result;
}
}  // namespace Kiran