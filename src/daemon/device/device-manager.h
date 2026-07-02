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

#include <QDBusContext>
#include <QDBusObjectPath>
#include <QMap>
#include <QObject>
#include "src/daemon/device/device-factory.h"
#include "src/daemon/device/device.h"
#include "src/daemon/device/sd-device-monitor.h"
#include "src/daemon/device/sd-device.h"

class DeviceManagerAdaptor;

namespace KS
{
class DeviceManager : public QObject,
                      protected QDBusContext
{
    Q_OBJECT
public:
    DeviceManager(QObject *parent);
    virtual ~DeviceManager();

public Q_SLOTS:  // METHODS
    // 获取所有设备信息
    QString GetDevices();

    // 获取特定设备信息
    QString GetDevice(const QString &id);

    // 修改权限
    bool ChangePermission(const QString &permissions);

    // 处理Udev事件
    void handleUdevEvent(SdDevice *device);

private:
    void init();
    void initDevices();
    void addDevice(SdDevice *device);
    QSharedPointer<Device> findDevice(const QString &id);
    void handleUdevAddEvent(SdDevice *device);
    void handleUdevRemoveEvent(SdDevice *device);
    void handleUdevChangeEvent(SdDevice *device);
    QJsonObject makeDevcieJsonInfo(QSharedPointer<Device> device);

private:
    DeviceManagerAdaptor *m_dbusAdaptor;
    QMap<QString, QSharedPointer<Device>> m_devMap;
    DeviceFactory m_devFactory;
    SdDeviceMonitor m_devMonitor;
};
}  // namespace KS
