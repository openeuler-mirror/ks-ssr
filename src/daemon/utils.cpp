/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#include "utils.h"
#include <ssr-marcos.h>

namespace KS
{
QString Utils::accountRoleEnum2Str(AccountRole role)
{
    switch (role)
    {
    case AccountRole::ACCOUNT_ROLE_SYSADMIN:
        return "sysadm";
    case AccountRole::ACCOUNT_ROLE_SECADMIN:
        return "secadm";
    case AccountRole::ACCOUNT_ROLE_AUDITADMIN:
        return "audadm";
    default:
        return "unknown";
    }
}

AccountRole Utils::accountRoleStr2Enum(const QString& roleStr)
{
    switch (shash(roleStr.toLatin1().data()))
    {
    case CONNECT("sysadm", _hash):
        return AccountRole::ACCOUNT_ROLE_SYSADMIN;
    case CONNECT("secadm", _hash):
        return AccountRole::ACCOUNT_ROLE_SECADMIN;
    case CONNECT("audadm", _hash):
        return AccountRole::ACCOUNT_ROLE_AUDITADMIN;
    default:
        return AccountRole::ACCOUNT_ROLE_NOACCOUNT;
    }
}

QString Utils::logTypeEnum2Str(LogType logType)
{
    switch (logType)
    {
    case LogType::DEVICE:
        return "DEVICE";
    case LogType::TOOL_BOX:
        return "TOOL_BOX";
    case LogType::BASELINE_REINFORCEMENT:
        return "BASELINE_REINFORCEMENT";
    case LogType::TRUSTED_PROTECTION:
        return "TRUSTED_PROTECTION";
    case LogType::FILES_PROTECTION:
        return "FILES_PROTECTION";
    case LogType::PRIVATE_BOX:
        return "PRIVATE_BOX";
    case LogType::ACCOUNT:
        return "ACCOUNT";
    case LogType::AVC:
        return "AVC";
    default:
        return "ERROR";
    }
}

LogType Utils::logTypeStr2Enum(const QString& logTypeStr)
{
    switch (shash(logTypeStr.toLatin1().data()))
    {
    case CONNECT("DEVICE", _hash):
        return LogType::DEVICE;
    case CONNECT("TOOL_BOX", _hash):
        return LogType::TOOL_BOX;
    case CONNECT("BASELINE_REINFORCEMENT", _hash):
        return LogType::BASELINE_REINFORCEMENT;
    case CONNECT("TRUSTED_PROTECTION", _hash):
        return LogType::TRUSTED_PROTECTION;
    case CONNECT("FILES_PROTECTION", _hash):
        return LogType::FILES_PROTECTION;
    case CONNECT("PRIVATE_BOX", _hash):
        return LogType::PRIVATE_BOX;
    case CONNECT("ACCOUNT", _hash):
        return LogType::ACCOUNT;
    case CONNECT("AVC", _hash):
        return LogType::AVC;
    default:
        return LogType::ERROR;
    }
}

}  // namespace KS
