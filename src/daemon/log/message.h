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

class Message : public QObject
{
    Q_OBJECT

public:
    // 此枚举标识日志消息属于哪个模块，审计日志不由此类写，但是前端拿日志数据时审计日志也会从此类的接口拿，所以在此定义审计类型。

    enum class LogType
    {
        ERROR = -1,
        AUDIT,
        DEVICE,
        LOG,
        SECURITY_BOX
    };
    Q_ENUM(LogType)

    Message();
    ~Message() = default;
    Message(const LogType type, const QString& logMsg, const QDateTime& timeStamp = QDateTime::currentDateTime());
    QString serialize(Qt::DateFormat format = Qt::ISODateWithMs) const;
    inline Message& deserialize(const QString& str);

    // Message 未被初始化时， isValid 函数返回 false
    bool isValid() const;

private:
    QDateTime m_timeStamp;
    LogType m_type;
    QString m_logMsg;
    // Message 未被初始化时， isValid 函数返回 false
    bool m_isValid;
    static QMetaEnum m_metaLogType;
    static const QString& m_separator;
};
};  // namespace Log
};  // namespace KS