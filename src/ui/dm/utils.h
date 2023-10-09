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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#pragma once

#include <QObject>
#include "include/ssr-i.h"

#define ENABLE QObject::tr("Enable")
#define DISABLE QObject::tr("Disable")
#define UNAUTHORIED QObject::tr("Unauthoried")
#define SUCCESSFUL QObject::tr("Successful")
#define FAILED QObject::tr("Failed")

namespace KS
{
namespace DM
{
class Utils : public QObject
{
    Q_OBJECT
public:
    static QString deviceTypeEnum2Str(DeviceType type);
    static QString interfaceTypeEnum2Str(InterfaceType type);
    static QString deviceStateEnum2Str(DeviceState state);
    static QString deviceConnectStateEnum2Str(DeviceConnectState state);

    static DeviceState deviceStateStr2Enum(const QString &state);
};
}  // namespace DM
}  // namespace KS
