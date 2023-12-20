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
    case LOG_TYPE_AUDIT:
        return tr("Audit log");
    case LOG_TYPE_TRUSTED:
        return tr("Trusted log");
    case LOG_TYPE_OTHER:
        return tr("Other log");
    default:
        break;
    }
    return QString();
}

}  // namespace Log
}  // namespace KS
