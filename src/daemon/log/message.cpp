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

#include "message.h"

namespace KS
{
namespace Log
{

QMetaEnum Message::m_metaLogType = QMetaEnum::fromType<Message::LogType>();
const QString& Log::Message::m_separator = "|";

Message::Message()
    : m_isValid(false)
{
}

Message::Message(const Message::LogType type, const QString& logMsg, const QDateTime& timeStamp)
    : m_timeStamp(timeStamp),
      m_type(type),
      m_logMsg(logMsg),
      m_isValid(true)
{
}

QString Message::serialize(Qt::DateFormat format) const
{
    return QString("%1%2%3%4%5").arg(m_timeStamp.toString(format), Message::m_separator, m_metaLogType.valueToKey(static_cast<int>(m_type)), Message::m_separator, m_logMsg);
}

inline Message& Message::deserialize(const QString& str)
{
    auto firstSp = str.indexOf(Message::m_separator);
    auto secondSp = str.indexOf(Message::m_separator, firstSp);
    if (firstSp == -1 || secondSp == -1)
    {
        m_isValid = false;
        KLOG_WARNING() << "failed to deserialize: " << str;
        return *this;
    }
    m_isValid = true;
    m_timeStamp = QDateTime::fromString(str.mid(0, firstSp));
    m_type = static_cast<Message::LogType>(m_metaLogType.keyToValue(str.mid(firstSp, secondSp).toLocal8Bit()));
    m_logMsg = str.mid(secondSp);

    return *this;
}

bool Message::isValid() const
{
    return m_isValid;
}
};  // namespace Log
};  // namespace KS