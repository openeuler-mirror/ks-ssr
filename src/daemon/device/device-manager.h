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
#include "src/daemon/device/device-interface.h"
#include "src/daemon/device/device.h"
#include "src/daemon/device/device-log.h"
#include "src/daemon/device/sd/sd-device-monitor.h"

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

    // 获取所有接口信息
    QString GetInterfaces();

    // 获取特定特定信息
    QString GetInterface(int type);

    // 修改权限
    bool ChangePermission(const QString &id,
                          const QString &permissions);

    // 启用设备
    bool Enable(const QString &id);

    // 禁用设备
    bool Disable(const QString &id);

    // 启用设备接口
    bool EnableInterface(int type);

    // 禁用设备接口
    bool DisableInterface(int type);

    // 获取连接记录
    QString GetRecords();

private slots:
    void handleUdevEvent(SDDevice *device,
                         int action);

private:
    void init();
    void initDevices();
    QSharedPointer<Device> findDevice(const QString &id);
    void addDevice(SDDevice *sdDevice);
    void handleUdevAddEvent(SDDevice *sdDevice);
    void handleUdevRemoveEvent(SDDevice *sdDevice);
    void handleUdevChangeEvent(SDDevice *sdDevice);
    void recordDeviceConnection(QSharedPointer<Device> device);

private:
    DeviceManagerAdaptor *m_dbusAdaptor;
    QMap<QString, QSharedPointer<Device>> m_devices;
    DeviceFactory m_devFactory;
    DeviceInterface m_devInterface;
    SDDeviceMonitor m_devMonitor;
    DeviceLog m_devLog;
};
}  // namespace KS
