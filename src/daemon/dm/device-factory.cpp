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
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#include "src/daemon/dm/device-factory.h"
#include <qt5-log-i.h>
#include <QMutex>
#include "src/daemon/dm/drm-device.h"
#include "src/daemon/dm/usb-device.h"
#include "ssr-marcos.h"

namespace KS
{
namespace DM
{
DeviceFactory::DeviceFactory(QObject* parent)
    : QObject{parent}
{
}

QSharedPointer<Device> DeviceFactory::createDevice(SDDevice* device)
{
    auto subsystem = device->getSubsystem();
    auto devtype = device->getDevtype();
    auto syspath = device->getSyspath();

    if (subsystem == "usb" && devtype == "usb_device")
    {
        return QSharedPointer<USBDevice>(new USBDevice(syspath));
    }
    else if (subsystem == "drm")
    {
        return QSharedPointer<DRMDevice>(new DRMDevice(syspath));
    }

    return nullptr;
}
}  // namespace DM
}  // namespace KS
