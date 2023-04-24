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
#pragma once

#include <systemd/sd-device.h>
#include <QObject>
#include <QSocketNotifier>
#include "src/daemon/device/sd-device.h"

namespace KS
{
class SdDeviceMonitor : public QObject
{
    Q_OBJECT

public:
    SdDeviceMonitor(QObject *parent = nullptr);
    virtual ~SdDeviceMonitor();
    void init();

signals:
    void udevSignal(SdDevice *device);

public slots:
    void sendUdevSignal(SdDevice *device);

private:
    void recivUdevMessage(int fd);

private:
    sd_device_monitor *m_monitor;
    sd_event *m_event;
    QSocketNotifier *m_sn;
};

}  // namespace KS