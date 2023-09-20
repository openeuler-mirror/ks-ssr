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

#include <QMap>
#include <QObject>
#include <QSocketNotifier>
#include "src/daemon/device/sd/sd-device.h"

typedef struct sd_device_monitor sd_device_monitor;
typedef struct sd_event sd_event;

namespace KS
{

enum SDDeviceAction
{
    SD_DEVICE_ACTION_INVALID = 0,
    SD_DEVICE_ACTION_ADD,
    SD_DEVICE_ACTION_REMOVE,
    SD_DEVICE_ACTION_CHANGE
};

class SDDeviceMonitor : public QObject
{
    Q_OBJECT

public:
    SDDeviceMonitor(QObject *parent = nullptr);
    virtual ~SDDeviceMonitor();
    void handleDeviceChange(sd_device *device);

public slots:
    static int deviceMonitorHandler(sd_device_monitor *monitor, sd_device *device, void *userdata);

signals:
    void deviceChanged(SDDevice *device, int action);

private:
    void init();
    void initDevices();
    bool isDeviceExisted(const QString &syspath);

private slots:
    void recivUdevMessage(int fd);

private:
    sd_device_monitor *m_monitor;
    sd_event *m_event;
    QSocketNotifier *m_socketNotify;
    QMap<QString, QSharedPointer<SDDevice>> m_devices;
};

}  // namespace KS