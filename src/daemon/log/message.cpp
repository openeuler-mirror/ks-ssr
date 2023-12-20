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

#include "src/daemon/log/message.h"
#include <qt5-log-i.h>
#include "src/daemon/account/manager.h"
#include "src/daemon/log/manager.h"

namespace KS
{
namespace Log
{

QMetaEnum Message::m_metaLogType = QMetaEnum::fromType<Manager::LogType>();
const QString& Message::m_separator = "|";

QString Message::serialize(const Log& log, Qt::DateFormat format)
{
    QStringList msg{};
    msg << Account::Manager::m_accountManager->m_metaAccountEnum.valueToKey((static_cast<int>(log.role)))
        << log.timeStamp.toString(format)
        << m_metaLogType.valueToKey(static_cast<int>(log.type))
        << QString(log.result ? "true" : "false")
        << log.logMsg;
    return msg.join(Message::m_separator);
}

Log Message::deserialize(const QString& str)
{
    auto log = str.split(Message::m_separator);
    // 判断日志中元素数量是否和现在的日志结构相等， 魔法数 5 是日志的属性数量。
    if (log.size() != 5)
    {
        KLOG_WARNING() << "Failed to deserialize log: " << str << ", skip this.";
        return Log{};
    }
    return {
        static_cast<Account::Manager::AccountRole>(
            Account::Manager::m_accountManager->m_metaAccountEnum.keyToValue(log.at(0).toLatin1())),
        QDateTime::fromString(log.at(1), Qt::ISODate),
        static_cast<Manager::LogType>(m_metaLogType.keyToValue(log.at(2).toLatin1())),
        log.at(3) == "true",
        log.at(4)};
}
};  // namespace Log
};  // namespace KS