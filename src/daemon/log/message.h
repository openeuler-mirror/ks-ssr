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

#include <QDateTime>
#include <QMetaEnum>
#include <QString>

namespace KS
{
namespace Log
{
struct Log;
class Message : QObject
{
    Q_OBJECT
public:
    Message() = delete;
    virtual ~Message(){};
    static QString serialize(const Log& log, Qt::DateFormat format = Qt::ISODateWithMs);
    static Log deserialize(const QString& str);

private:
    // Message 未被初始化时， isValid 函数返回 false
    static QMetaEnum m_metaLogType;
    static const QString& m_separator;
};
};  // namespace Log
};  // namespace KS