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

#include "src/daemon/dm/sd/sd-device-monitor.h"
#include <qt5-log-i.h>
#include <systemd/sd-device.h>
#include "src/daemon/dm/sd/sd-device-enumerator.h"

namespace KS
{
namespace DM
{
#define UDEV_BUFFER_SIZE (128 * 1024 * 1024)

SDDeviceMonitor::SDDeviceMonitor(QObject *parent)
    : QObject(parent),
      m_monitor(nullptr),
      m_event(nullptr),
      m_socketNotify(nullptr)
{
    this->init();
}

SDDeviceMonitor::~SDDeviceMonitor()
{
    if (m_monitor)
    {
        sd_device_monitor_unref(m_monitor);
        m_monitor = nullptr;
    }

    if (m_event)
    {
        sd_event_unref(m_event);
        m_event = nullptr;
    }

    delete m_socketNotify;
    m_socketNotify = nullptr;
}

void SDDeviceMonitor::handleDeviceChange(sd_device *device)
{
    SDDevice sdDevice(device);
    QString syspath = sdDevice.getSyspath();

    if (this->isDeviceExisted(syspath))
    {
        if (!m_devices.value(syspath))
        {
            m_devices.insert(syspath, QSharedPointer<SDDevice>(new SDDevice(syspath)));
            Q_EMIT this->deviceChanged(&sdDevice, SD_DEVICE_ACTION_ADD);
        }
        else
        {
            Q_EMIT this->deviceChanged(&sdDevice, SD_DEVICE_ACTION_CHANGE);
        }
    }
    else
    {
        Q_EMIT this->deviceChanged(&sdDevice, SD_DEVICE_ACTION_REMOVE);
        m_devices.remove(syspath);
    }
}

int SDDeviceMonitor::deviceMonitorHandler(sd_device_monitor *sdMonitor,
                                          sd_device *device,
                                          void *userdata)
{
    SDDeviceMonitor *monitor = (SDDeviceMonitor *)userdata;

    monitor->handleDeviceChange(device);

    return 0;
}

bool SDDeviceMonitor::isDeviceExisted(const QString &syspath)

{
    SDDeviceEnumerator enumerator;
    auto devices = enumerator.getDevices();

    Q_FOREACH (auto device, devices)
    {
        if (device->getSyspath() == syspath)
        {
            return true;
        }
    }

    return false;
}

void SDDeviceMonitor::recivUdevMessage(int fd)
{
    sd_event_run(m_event, -1);
}

void SDDeviceMonitor::init()
{
    auto ret = sd_event_default(&m_event);
    if (ret < 0)
    {
        KLOG_ERROR() << "Failed to get default event.";
        return;
    }

    ret = sd_device_monitor_new(&m_monitor);
    if (ret < 0)
    {
        KLOG_ERROR() << "Failed to create device monitor.";
        return;
    }

    ret = sd_device_monitor_set_receive_buffer_size(m_monitor, UDEV_BUFFER_SIZE);
    if (ret < 0)
    {
        KLOG_ERROR() << "Failed to set device moinitor receive buffer size.";
        return;
    }

    ret = sd_device_monitor_attach_event(m_monitor, m_event);
    if (ret < 0)
    {
        KLOG_ERROR() << "Failed to attach event to device monitor.";
        return;
    }

    ret = sd_device_monitor_start(m_monitor, deviceMonitorHandler, this);
    if (ret < 0)
    {
        KLOG_ERROR() << "Failed to start device monitor.";
        return;
    }

    m_socketNotify = new QSocketNotifier(sd_event_get_fd(m_event), QSocketNotifier::Read);
    m_socketNotify->setEnabled(true);
    QObject::connect(m_socketNotify, &QSocketNotifier::activated, this, &SDDeviceMonitor::recivUdevMessage);
}

void SDDeviceMonitor::initDevices()
{
    SDDeviceEnumerator enumerator;
    auto devices = enumerator.getDevices();

    Q_FOREACH (auto device, devices)
    {
        QString syspath = device->getSyspath();

        if (syspath.isNull())
        {
            continue;
        }

        m_devices.insert(syspath, QSharedPointer<SDDevice>(new SDDevice(syspath)));
    }
}
}  // namespace DM
}  // namespace KS