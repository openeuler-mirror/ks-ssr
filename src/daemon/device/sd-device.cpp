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

#include "src/daemon/device/sd-device.h"
#include <qt5-log-i.h>
#include "sc-marcos.h"

namespace KS
{
SdDevice::SdDevice(sd_device* device, QObject* parent)
    : QObject(parent),
      m_device(nullptr)
{
    m_device = sd_device_ref(device);
}

SdDevice::SdDevice(const QString& syspath, QObject* parent)
    : QObject(parent),
      m_device(nullptr)
{
    RETURN_IF_TRUE(syspath.isNull())

    auto ret = sd_device_new_from_syspath(&m_device, syspath.toStdString().c_str());
    if (ret < 0)
    {
        KLOG_ERROR() << "Failed to create device from syspath: " << syspath;
    }
}

SdDevice::~SdDevice()
{
    if (m_device)
    {
        sd_device_unref(m_device);
    }
}

QString SdDevice::get_syspath() const
{
    QString result;
    const char* syspath = NULL;

    auto ret = sd_device_get_syspath(m_device, &syspath);
    if (ret < 0)
    {
        KLOG_DEBUG() << "Failed to get device syspath.";
        return result;
    }

    result.append(syspath);

    return result;
}

QString SdDevice::get_subsystem() const
{
    QString result;
    const char* subsystem = NULL;

    auto ret = sd_device_get_subsystem(m_device, &subsystem);
    if (ret < 0)
    {
        KLOG_DEBUG() << "Failed to get device subsystem.";
        return result;
    }

    result.append(subsystem);

    return result;
}

QString SdDevice::get_devtype() const
{
    QString result;
    const char* devtype = NULL;

    auto ret = sd_device_get_devtype(m_device, &devtype);
    if (ret < 0)
    {
        KLOG_DEBUG() << "Failed to get device devtype.";
        return result;
    }

    result.append(devtype);

    return result;
}

QString SdDevice::get_sysname() const
{
    QString result;
    const char* sysname = NULL;

    auto ret = sd_device_get_sysname(m_device, &sysname);
    if (ret < 0)
    {
        KLOG_DEBUG() << "Failed to get device sysname.";
        return result;
    }

    result.append(sysname);

    return result;
}

QString SdDevice::get_sysattr_value(const QString& attr) const
{
    QString result;
    const char* value = NULL;

    RETURN_VAL_IF_TRUE(attr.isNull(), result)

    auto ret = sd_device_get_sysattr_value(m_device, attr.toStdString().c_str(), &value);
    if (ret < 0)
    {
        KLOG_DEBUG() << "Failed to get device sysattr: " << attr;
        return result;
    }

    result.append(value);

    return result;
}

int SdDevice::get_action() const
{
    auto result = SD_DEVICE_ACTION_INVALID;
    auto action = _SD_DEVICE_ACTION_INVALID;

    auto ret = sd_device_get_action(m_device, &action);
    if (ret < 0)
    {
        KLOG_DEBUG() << "Failed to get syspath of device action.";
        return action;
    }

    switch (action)
    {
    case SD_DEVICE_ADD:
        result = SD_DEVICE_ACTION_ADD;
        break;

    case SD_DEVICE_REMOVE:
        result = SD_DEVICE_ACTION_REMOVE;
        break;

    case SD_DEVICE_CHANGE:
        result = SD_DEVICE_ACTION_CHANGE;
        break;

    default:
        break;
    }

    return result;
}

void SdDevice::trigger()
{
    if (sd_device_trigger(m_device, SD_DEVICE_CHANGE) < 0)
    {
        KLOG_ERROR() << "Failed to trigger sd device with change event.";
    }
}
}  // namespace KS