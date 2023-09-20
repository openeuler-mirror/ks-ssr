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

#include "src/daemon/device/sd/sd-device.h"
#include <qt5-log-i.h>
#include <systemd/sd-device.h>
#include "sc-marcos.h"

namespace KS
{
SDDevice::SDDevice(sd_device* device, QObject* parent)
    : QObject(parent),
      m_device(nullptr)
{
    m_device = sd_device_ref(device);
}

SDDevice::SDDevice(const QString& syspath, QObject* parent)
    : QObject(parent),
      m_device(nullptr)
{
    RETURN_IF_TRUE(syspath.isNull())

    sd_device_new_from_syspath(&m_device, syspath.toStdString().c_str());
}

SDDevice::~SDDevice()
{
    if (m_device)
    {
        sd_device_unref(m_device);
    }
}

QString SDDevice::getSyspath() const
{
    const char* syspath = NULL;

    RETURN_VAL_IF_TRUE(sd_device_get_syspath(m_device, &syspath) < 0, QString())

    return QString(syspath);
}

QString SDDevice::getSubsystem() const
{
    const char* subsystem = NULL;

    RETURN_VAL_IF_TRUE(sd_device_get_subsystem(m_device, &subsystem) < 0, QString())

    return QString(subsystem);
}

QString SDDevice::getDevtype() const
{
    const char* devtype = NULL;

    RETURN_VAL_IF_TRUE(sd_device_get_devtype(m_device, &devtype) < 0, QString())

    return QString(devtype);
}

QString SDDevice::getSysname() const
{
    const char* sysname = NULL;

    RETURN_VAL_IF_TRUE(sd_device_get_sysname(m_device, &sysname) < 0, QString())

    return QString(sysname);
}

QString SDDevice::getSysattrValue(const QString& attr) const
{
    const char* value = NULL;

    RETURN_VAL_IF_TRUE(attr.isNull(), QString())

    auto ret = sd_device_get_sysattr_value(m_device, attr.toStdString().c_str(), &value);
    if (ret < 0)
    {
        KLOG_DEBUG() << "Failed to get device sysattr: " << attr;
        return QString();
    }

    return QString(value);
}

void SDDevice::trigger()
{
    if (sd_device_trigger(m_device, SD_DEVICE_CHANGE) < 0)
    {
        KLOG_ERROR() << "Failed to trigger sd device with change event.";
    }
}
}  // namespace KS