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

#pragma once

#include <QMap>
#include <QObject>
#include "src/daemon/dm/device-log.h"
#include "src/daemon/dm/device-mount-monitor.h"
#include "src/daemon/dm/device.h"
#include "src/daemon/dm/sd/sd-device-monitor.h"

namespace KS
{
namespace DM
{
class DeviceFactory;
class DBus;

class DeviceManager : public QObject
{
    Q_OBJECT

public:
    static void globalInit(QObject *parent);
    static void globalDeinit();

    static DeviceManager *instance()
    {
        return m_instance;
    };

public:
    // 获取所有设备信息
    DeviceList getDevices();
    DeviceList getDevicesByInterface(int interfaceType);
    // 获取特定设备信息
    QSharedPointer<Device> getDevice(const QString &syspath)
    {
        return m_devices.value(syspath);
    };
    // FIXME: 需要处理ID重复的情况
    QSharedPointer<Device> getDeviceByID(const QString &id);
    // 获取设备日志对象
    QSharedPointer<DeviceLog> getDeviceLog()
    {
        return m_deviceLog;
    };

    // 检查设备挂载权限
    void checkDeviceMount(const QSharedPointer<Device> device);

    // 重放接口设备事件
    void triggerInterfaceDevices(int interfaceType);

    // 是否支持HDMI关闭
    bool isSupportHDMIDisable();

private:
    DeviceManager(QObject *parent);
    virtual ~DeviceManager();

private Q_SLOTS:
    void handleUdevEvent(SDDevice *device,
                         int action);
    void handleMountEvent(const DeviceMount *mount);

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
    QSharedPointer<Device> getParentDevice(const QString syspath) const;
    bool isDeviceMountPerChanged(const QSharedPointer<Device> device,
                                 const DeviceMount *mount);
    void remountDevice(const QSharedPointer<Device> device,
                       const DeviceMount *mount);
    QString getMountSyspath(const DeviceMount *mount);

public:
    static QString deviceTypeEnum2Str(int deviceType);
    static QString interfaceTypeEnum2Str(int interfaceType);

private:
    static DeviceManager *m_instance;
    // <syspath, device>
    QMap<QString, QSharedPointer<Device>> m_devices;
    DeviceFactory *m_deviceFactory;
    SDDeviceMonitor m_sdDeviceMonitor;
    DBus *m_deviceDBus;
    QSharedPointer<DeviceLog> m_deviceLog;
    DeviceMountMonitor m_mountMonitor;
};
}  // namespace DM
}  // namespace KS
