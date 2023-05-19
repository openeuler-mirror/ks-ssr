/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
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

#include "src/daemon/device/device-factory.h"
#include <qt5-log-i.h>
#include <QMutex>
#include "ksc-marcos.h"
#include "src/daemon/device/drm-device.h"
#include "src/daemon/device/usb-device.h"

namespace KS
{
DeviceFactory::DeviceFactory(QObject* parent) : QObject{parent}
{
}

QSharedPointer<Device> DeviceFactory::createDevice(SDDevice* device)
{
    auto subsystem = device->getSubsystem();
    auto devtype = device->getDevtype();
    auto syspath = device->getSyspath();

    KLOG_DEBUG() << "create device for subsystem " << subsystem << ", syspath: " << syspath;

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
}  // namespace KS
