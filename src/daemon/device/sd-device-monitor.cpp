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

#include "src/daemon/device/sd-device-monitor.h"
#include <qt5-log-i.h>

namespace KS
{
#define UDEV_BUFFER_SIZE (128 * 1024 * 1024)

SdDeviceMonitor::SdDeviceMonitor(QObject *parent)
    : QObject(parent),
      m_monitor(nullptr),
      m_event(nullptr),
      m_sn(nullptr)
{
    this->init();
}

SdDeviceMonitor::~SdDeviceMonitor()
{
    if (m_monitor)
    {
        sd_device_monitor_unref(m_monitor);
    }

    if (m_event)
    {
        sd_event_unref(m_event);
    }

    if (m_sn)
    {
        m_sn->deleteLater();
    }
}

static int deviceMonitorHandler(sd_device_monitor *m, sd_device *device, void *userdata)
{
    auto *monitor = (SdDeviceMonitor *)userdata;
    SdDevice sdDevice (device);

    monitor->sendUdevSignal(&sdDevice);

    return 0;
}

void SdDeviceMonitor::recivUdevMessage(int fd)
{
    sd_event_run(m_event, -1);
}

void SdDeviceMonitor::init()
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

    m_sn = new QSocketNotifier(sd_event_get_fd(m_event), QSocketNotifier::Read);
    m_sn->setEnabled(true);
    QObject::connect(m_sn, &QSocketNotifier::activated, this, &SdDeviceMonitor::recivUdevMessage);
}

void SdDeviceMonitor::sendUdevSignal(SdDevice *device)
{
    Q_EMIT udevSignal(device);
}

}  // namespace KS