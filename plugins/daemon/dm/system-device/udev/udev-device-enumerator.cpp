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

#include "udev-device-enumerator.h"
#include <libudev.h>
#include <qt5-log-i.h>
#include <QList>
#include "udev-device.h"

struct udev* udev = udev_new();

namespace KS
{
namespace DM
{
UdevDeviceEnumerator::UdevDeviceEnumerator()
{
    m_deviceEnum = udev_enumerate_new(udev);
    if (udev_enumerate_scan_devices(m_deviceEnum))
    {
        KLOG_ERROR() << "Failed to init device enumerator";
    }
    auto udevEntry = udev_enumerate_get_list_entry(m_deviceEnum);
    while (udevEntry)
    {
        auto device = udev_device_new_from_syspath(udev, udev_list_entry_get_name(udevEntry));
        m_devices.append(new UdevDevice{device});
        // 因为构造了 UdevDevice 会使 device 的引用计数加一， 所以这里需要 unref 一下。
        udev_device_unref(device);
        udevEntry = udev_list_entry_get_next(udevEntry);
    }
}
UdevDeviceEnumerator::~UdevDeviceEnumerator()
{
    udev_enumerate_unref(m_deviceEnum);
    for (auto device : m_devices)
    {
        delete device;
    }
}

QList<UdevDevice*> UdevDeviceEnumerator::getDevices() const
{
    return m_devices;
}

}  // namespace DM
}  // namespace KS
