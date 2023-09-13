/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#include "lib/base/str-utils.h"
#include <ssr-marcos.h>
#include <algorithm>
#include "qt5-log-i.h"

namespace KS
{
QVector<QString> StrUtils::splitLines(const QString &s)
{
    QVector<QString> ret;
    size_t i = 0;
    size_t line_start = 0;
    while (i < s.length())
    {
        if (s[static_cast<uint32_t>(i)] == '\n')
        {
            ret.push_back(s.mid(line_start, i - line_start));
            i++;
            line_start = i;
        }
        else if (s[static_cast<uint32_t>(i)] == '\r')
        {
            if ((i + 1 < s.length() && s[static_cast<uint32_t>(i + 1)] != '\n') ||
                i + 1 >= s.length())
            {
                ret.push_back(s.mid(line_start, i - line_start));
                i++;
                line_start = i;
            }
            else  // if (i + 1 < s.length() && s[i + 1] == '\n')
            {
                ret.push_back(s.mid(line_start, i - line_start));
                i += 2;
                line_start = i;
            }
        }
        else
        {
            i++;
        }
    }
    if (line_start < s.length())
    {
        ret.push_back(s.mid(line_start, s.length() - line_start));
    }
    return ret;
}

QString StrUtils::tolower(const QString &str)
{
    // 为了解决编译错误 `cannot convert ‘QChar’ to ‘int’ in argument passing`
    std::string new_str(str.toStdString());
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), ::tolower);
    return QString::fromStdString(new_str);
}

QString StrUtils::toupper(const QString &str)
{
    // 为了解决编译错误 `cannot convert ‘QChar’ to ‘int’ in argument passing`
    std::string new_str(str.toStdString());
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), ::toupper);
    return QString::fromStdString(new_str);
}

QVector<QString> StrUtils::splitWithChar(const QString &s, char delimiter, bool is_merge_delimiter)
{
    QVector<QString> v;
    size_t start = 0;
    size_t i = 0;
    while (i < s.length())
    {
        if (delimiter == s[static_cast<uint32_t>(i)])
        {
            if (i > start || !is_merge_delimiter)
            {
                v.push_back(s.mid(start, i - start));
            }
            i++;
            start = i;
        }
        else
        {
            i++;
        }
    }
    v.push_back(s.mid(start, s.length() - start));
    return v;
}

static bool is_space(char c)
{
    return (std::isspace(c) == 0);
}

QString StrUtils::ltrim(const QString &s)
{
#pragma message("容易出问题,建议多测以下")
    auto tmp = s.toStdString();
    auto iter = std::find_if(tmp.begin(), tmp.end(), is_space);
    return QString::fromStdString(std::string(iter, tmp.end()));
}

QString StrUtils::rtrim(const QString &s)
{
    auto tmp = s.toStdString();
    auto iter = std::find_if(tmp.rbegin(), tmp.rend(), is_space);
    return QString::fromStdString(std::string(tmp.begin(), iter.base()));
}

QString StrUtils::trim(const QString &s)
{
    return StrUtils::ltrim(StrUtils::rtrim(s));
}

QString StrUtils::json2str(const QJsonObject &json)
{
    QJsonObject jsonObject;
    jsonObject["indentation"] = json;
    return QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
}

QJsonValue StrUtils::str2json(const QString &str)
{
    // str.toLatin1().data() 这种做法是错误的，中间对象 QByteArray 会被析构。
    auto ByteArray = str.toLatin1();
    KLOG_DEBUG() << "json str: " << str;
    QVariant variant(str);

    return QJsonValue::fromVariant(variant);
}

bool StrUtils::startswith(const QString &str, const QString &prefix)
{
    RETURN_VAL_IF_TRUE(str.size() < prefix.size(), false);
    return (str.mid(0, prefix.size()) == prefix);
}

QVector<QString> StrUtils::intersect(const QVector<QString> &a1, const QVector<QString> &a2)
{
    QVector<QString> result;

    for (uint32_t i = 0; i < a1.size(); ++i)
    {
        for (uint32_t j = 0; j < a2.size(); ++j)
        {
            if (a1[i] == a2[j])
            {
                result.push_back(a1[i]);
            }
        }
    }

    return result;
}
}  // namespace KS