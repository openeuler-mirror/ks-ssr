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
#include "src/daemon/device/device-log.h"
#include "src/daemon/device/device.h"
#include "src/daemon/device/sd/sd-device-monitor.h"

namespace KS
{
class DeviceFactory;
class DeviceDBus;

class DeviceManager : public QObject
{
    Q_OBJECT

public:
    static void globalInit(QObject *parent);
    static void globalDeinit();

    static DeviceManager *instance() { return m_instance; };

public:
    // 获取所有设备信息
    DeviceList getDevices() { return this->m_devices.values(); };
    DeviceList getDevicesByInterface(int interfaceType);
    // 获取特定设备信息
    QSharedPointer<Device> getDevice(const QString &syspath) { return m_devices.value(syspath); };
    // FIXME: 需要处理ID重复的情况
    QSharedPointer<Device> getDeviceByID(const QString &id);
    // 获取设备日志对象
    QSharedPointer<DeviceLog> getDeviceLog() { return m_deviceLog; };

private:
    DeviceManager(QObject *parent);
    virtual ~DeviceManager();

private Q_SLOTS:
    void handleUdevEvent(SDDevice *device,
                         int action);

Q_SIGNALS:
    void deviceChanged(const QString &id, int action);

private:
    void init();
    void initDevices();
    void addDevice(SDDevice *sdDevice);
    void handleUdevAddEvent(SDDevice *sdDevice);
    void handleUdevRemoveEvent(SDDevice *sdDevice);
    void handleUdevChangeEvent(SDDevice *sdDevice);
    void recordDeviceConnection(QSharedPointer<Device> device);

private:
    static DeviceManager *m_instance;
    // <syspath, device>
    QMap<QString, QSharedPointer<Device>> m_devices;
    DeviceFactory *m_deviceFactory;
    SDDeviceMonitor m_sdDeviceMonitor;
    DeviceDBus *m_deviceDBus;
    QSharedPointer<DeviceLog> m_deviceLog;
};
}  // namespace KS
