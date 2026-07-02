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

#include "src/daemon/device/sd-device-enumerator.h"
#include <qt5-log-i.h>

namespace KS
{
SdDeviceEnumerator::SdDeviceEnumerator(QObject* parent)
    : QObject(parent),
      m_enumerator(nullptr)
{
    this->init();
}

SdDeviceEnumerator::~SdDeviceEnumerator()
{
    if (m_enumerator)
    {
        sd_device_enumerator_unref(m_enumerator);
    }
}

void SdDeviceEnumerator::init()
{
    auto ret = sd_device_enumerator_new(&m_enumerator);

    if (ret < 0)
    {
        KLOG_ERROR() << "Failed to create device enumerator.";
        return;
    }

    ret = sd_device_enumerator_add_match_subsystem(m_enumerator, "usb", true);
    if (ret < 0)
    {
        KLOG_ERROR() << "Failed to add match subsystem usb.";
    }

    ret = sd_device_enumerator_allow_uninitialized(m_enumerator);
    if (ret < 0)
    {
        KLOG_ERROR() << "Failed to allow uninitialized.";
    }
}

QList<QSharedPointer<SdDevice>> SdDeviceEnumerator::getDevices()
{
    QList<QSharedPointer<SdDevice>> list;

    for (auto device = sd_device_enumerator_get_device_first(m_enumerator);
         device != nullptr;
         device = sd_device_enumerator_get_device_next(m_enumerator))
    {
        auto newDevice = QSharedPointer<SdDevice>(new SdDevice(device));

        list.append(newDevice);
    }

    return list;
}

}  // namespace KS