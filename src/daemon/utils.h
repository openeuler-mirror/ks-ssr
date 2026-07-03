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

#pragma once

#include <daemon-log-i.h>
#include <ssr-i.h>
#include <QString>

namespace KS
{
class Utils
{
public:
    static QString accountRoleEnum2Str(AccountRole role);
    static AccountRole accountRoleStr2Enum(const QString& roleStr);

    static QString logTypeEnum2Str(LogType logType);
    static LogType logTypeStr2Enum(const QString& logTypeStr);
};

}  // namespace KS
