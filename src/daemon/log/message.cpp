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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#include "message.h"
#include <daemon-accounts-i.h>
#include <daemon-log-i.h>
#include <qt5-log-i.h>
#include "log-manager.h"

namespace KS
{
namespace Log
{
// QMetaEnum Message::m_metaLogType = QMetaEnum::fromType<Manager::LogType>();
const QString& Message::m_separator = "|";

QString Message::serialize(const LogRecord& log, Qt::DateFormat format)
{
    QStringList msg{};
    // 现版本不对外保暴露 userName 字段， 所以序列化时不序列化 userName
    msg << g_accountsManager->accountRoleEnum2Str(AccountRole(log.role))
        << log.timeStamp.toString(format)
        << Manager::logTypeEnum2Str(log.type)
        << QString(log.result ? "true" : "false")
        << log.logMsg;
    return msg.join(Message::m_separator);
}

LogRecord Message::deserialize(const QString& str)
{
    auto log = str.split(Message::m_separator);
    // 判断日志中元素数量是否和现在的日志结构相等， 魔法数 6 是日志的属性数量。
    if (log.size() != 5)
    {
        KLOG_WARNING() << "Failed to deserialize log: " << str << ", skip this.";
        return LogRecord{};
    }
    /// @note 这个版本不对外暴露 name 字段， name 字段的初始化统一用 role 的枚举 key
    return LogRecord{
        .name = log.at(0),
        .role = int(g_accountsManager->accountRoleStr2Enum(log.at(0))),
        .timeStamp = QDateTime::fromString(log.at(1), Qt::ISODate),
        .type = Manager::logTypeStr2Enum(log.at(2)),
        .result = log.at(3) == "true",
        .logMsg = log.at(4)};
}
};  // namespace Log
};  // namespace KS