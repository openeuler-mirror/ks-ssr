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

#pragma once

#include <fmt/format.h>
#include <QString>
#include <QVector>
#include <vector>
#include <QJsonDocument>
#include <QJsonObject>

namespace KS
{
class StrUtils
{
    static QVector<QString> v;

public:
    StrUtils(){};
    virtual ~StrUtils(){};

    static QVector<QString> splitLines(const QString &s);
    static QString tolower(const QString &str);
    static QString toupper(const QString &str);
    static QVector<QString> splitWithChar(const QString &s, char delimiter, bool is_merge_delimiter = false);

    // 去掉字符串前面的空白字符
    static QString ltrim(const QString &s);
    // 去掉字符串后面的空白字符
    static QString rtrim(const QString &s);
    // 去掉字符串前后的空白字符
    static QString trim(const QString &s);

    static QString json2str(const QJsonObject &json);
    static QJsonValue str2json(const QString &str);

    // 判断str是否以prefix字符串开头
    static bool startswith(const QString &str, const QString &prefix);

    // 字符串列表交集
    static QVector<QString> intersect(const QVector<QString> &a1, const QVector<QString> &a2);
};
}  // namespace KS