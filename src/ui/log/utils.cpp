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
 * Author:     chendingjian <chendingjian@kylinos.com.cn>
 */

#include "utils.h"
#include <qt5-log-i.h>
#include <QDateTime>
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/log/table.h"

namespace KS
{
namespace Log
{
QString Utils::logTypeEnum2Str(uint type)
{
    switch (type)
    {
    case LOG_TYPE_DEVICE:
        return tr("Device log");
        //    case LOG_TYPE_AUDIT:
        //        return tr("Audit log");
        //    case LOG_TYPE_LOG:
        //        return tr("Log");
    case LOG_TYPE_TOOL_BOX:
        return tr("Tool log");
    default:
        break;
    }
    return QString();
}

int Utils::str2LogTypeEnum(const QString &typeStr)
{
    if (typeStr == tr("Device log"))
    {
        return LOG_TYPE_DEVICE;
    }
    else if (typeStr == tr("Tool log"))
    {
        return LOG_TYPE_TOOL_BOX;
    }
    else
    {
        return -1;
    }
}

QString Utils::accountRoleEnum2Str(uint role)
{
    switch (role)
    {
    case ACCOUNT_ROLE_SYSADMIN:
        return tr("Sysadm");
    case ACCOUNT_ROLE_SECADMIN:
        return tr("Secadm");
    case ACCOUNT_ROLE_AUDITADMIN:
        return tr("Audadm");
    default:
        return tr("Unknown");
    }
}

int Utils::str2AccountRoleEnum(const QString &roleStr)
{
    if (roleStr == tr("Sysadm"))
    {
        return ACCOUNT_ROLE_SYSADMIN;
    }
    else if (roleStr == tr("Secadm"))
    {
        return ACCOUNT_ROLE_SECADMIN;
    }
    else if (roleStr == tr("Audadm"))
    {
        return ACCOUNT_ROLE_AUDITADMIN;
    }
    else
    {
        return -1;
    }
}

void Utils::deserialize(const QStringList &logs, QList<LogInfo> &logInfos)
{
    for (auto log : logs)
    {
        auto logItems = log.split("|");
        CONTINUE_IF_TRUE(log.size() != 5);
        auto logInfo = LogInfo{
            .type = LogType(logStrType2Enum(logItems.at(2))),
            .role = AccountRole(roleStrType2Enum(logItems.at(0))),
            .dataTime = QDateTime::fromString(logItems.at(1), Qt::ISODate).toString("yyyy/MM/dd HH:mm"),
            .message = logItems.at(4),
            .result = logItems.at(3) == "true"};
        logInfos << logInfo;
    }
}

int Utils::logStrType2Enum(const QString &logStr)
{
    if (logStr == "TOOL_BOX")
    {
        return LOG_TYPE_TOOL_BOX;
    }
    else if (logStr == "DEVICE")
    {
        return LOG_TYPE_DEVICE;
    }
    //    else if (logStr == "LOG")
    //    {
    //        return LOG_TYPE_LOG;
    //    }
    //    else if (logStr == "AUDIT")
    //    {
    //        return LOG_TYPE_AUDIT;
    //    }
    else
    {
        return -1;
    }
}

int Utils::roleStrType2Enum(const QString &roleStr)
{
    if (roleStr == "SYSADMIN")
    {
        return ACCOUNT_ROLE_SYSADMIN;
    }
    else if (roleStr == "SECADMIN")
    {
        return ACCOUNT_ROLE_SECADMIN;
    }
    else if (roleStr == "AUDITADMIN")
    {
        return ACCOUNT_ROLE_AUDITADMIN;
    }
    else if (roleStr == "NOACCOUNT")
    {
        return ACCOUNT_ROLE_NOACCOUNT;
    }
    else
    {
        return -1;
    }
}

}  // namespace Log
}  // namespace KS
