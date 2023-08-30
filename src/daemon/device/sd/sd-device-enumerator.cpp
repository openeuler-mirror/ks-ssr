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

#include "src/daemon/device/sd/sd-device-enumerator.h"
#include <qt5-log-i.h>
#include <systemd/sd-device.h>

namespace KS
{
SDDeviceEnumerator::SDDeviceEnumerator(QObject* parent)
    : QObject(parent),
      m_enumerator(nullptr)
{
    this->init();
}

SDDeviceEnumerator::~SDDeviceEnumerator()
{
    Q_FOREACH (auto device, m_devices)
    {
        delete device;
    }

    if (m_enumerator)
    {
        sd_device_enumerator_unref(m_enumerator);
    }
}

void SDDeviceEnumerator::init()
{
    if (sd_device_enumerator_new(&m_enumerator) < 0)
    {
        KLOG_ERROR() << "Failed to create device enumerator.";
        return;
    }

    if (sd_device_enumerator_allow_uninitialized(m_enumerator) < 0)
    {
        KLOG_ERROR() << "Failed to allow uninitialized.";
    }

    for (auto device = sd_device_enumerator_get_device_first(m_enumerator);
         device != nullptr;
         device = sd_device_enumerator_get_device_next(m_enumerator))
    {
        m_devices.append(new SDDevice(device));
    }
}

QList<SDDevice*> SDDeviceEnumerator::getDevices() const
{
    return m_devices;
}

}  // namespace KS