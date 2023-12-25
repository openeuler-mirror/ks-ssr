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
        return tr("Tool box log");
    default:
        break;
    }
    return QString();
}

int Utils::str2LogTypeEnum(const QString &typeStr)
{
    RETURN_VAL_IF_TRUE(typeStr == tr("Device log"), LOG_TYPE_DEVICE);
    RETURN_VAL_IF_TRUE(typeStr == tr("Tool box log"), LOG_TYPE_TOOL_BOX);
    return -1;
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
    RETURN_VAL_IF_TRUE(roleStr == tr("Sysadm"), ACCOUNT_ROLE_SYSADMIN);
    RETURN_VAL_IF_TRUE(roleStr == tr("Secadm"), ACCOUNT_ROLE_SECADMIN);
    RETURN_VAL_IF_TRUE(roleStr == tr("Audadm"), ACCOUNT_ROLE_AUDITADMIN);
    return -1;
}

void Utils::deserialize(const QStringList &logs, QList<LogInfo> &logInfos)
{
    for (auto& log : logs)
    {
        auto logItems = log.split("|");
        CONTINUE_IF_TRUE(logItems.size() != 5);
        auto logInfo = LogInfo{
            .type = static_cast<LogType>(logStrType2Enum(logItems.at(2))),
            .role = static_cast<AccountRole>(roleStrType2Enum(logItems.at(0))),
            .dataTime = QDateTime::fromString(logItems.at(1), Qt::ISODate).toString("yyyy/MM/dd HH:mm"),
            .message = logItems.at(4),
            .result = logItems.at(3) == "true"};
        logInfos << logInfo;
    }
}

int Utils::logStrType2Enum(const QString &logStr)
{
    RETURN_VAL_IF_TRUE(logStr == "TOOL_BOX", LOG_TYPE_TOOL_BOX);
    RETURN_VAL_IF_TRUE(logStr == "DEVICE", LOG_TYPE_DEVICE);
    //    else if (logStr == "LOG")
    //    {
    //        return LOG_TYPE_LOG;
    //    }
    //    else if (logStr == "AUDIT")
    //    {
    //        return LOG_TYPE_AUDIT;
    //    }
    return -1;
}

int Utils::roleStrType2Enum(const QString &roleStr)
{
    RETURN_VAL_IF_TRUE(roleStr == "sysadm", ACCOUNT_ROLE_SYSADMIN);
    RETURN_VAL_IF_TRUE(roleStr == "secadm", ACCOUNT_ROLE_SECADMIN);
    RETURN_VAL_IF_TRUE(roleStr == "audadm", ACCOUNT_ROLE_AUDITADMIN);
    RETURN_VAL_IF_TRUE(roleStr == "unknown_account", ACCOUNT_ROLE_NOACCOUNT);
    return -1;
}

}  // namespace Log
}  // namespace KS
