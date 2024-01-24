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
    // 现版本不对外保暴露 userName 字段， 所以序列化时不序列化 userName
    // msg << log.name
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
    // 判断日志中元素数量是否和现在的日志结构相等， 魔法数 6 是日志的属性数量。
    if (log.size() != 6)
    {
        KLOG_WARNING() << "Failed to deserialize log: " << str << ", skip this.";
        return Log{};
    }
    auto role = static_cast<Account::Manager::AccountRole>(
        Account::Manager::m_accountManager->m_metaAccountEnum.keyToValue(log.at(1).toLatin1()));
    /// @note 这个版本不对外暴露 name 字段， name 字段的初始化统一用 role 的枚举 key
    return {
        log.at(1),
        role,
        QDateTime::fromString(log.at(2), Qt::ISODate),
        static_cast<Manager::LogType>(m_metaLogType.keyToValue(log.at(3).toLatin1())),
        log.at(4) == "true",
        log.at(5)};
}
};  // namespace Log
};  // namespace KS