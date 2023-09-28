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
#include <QObject>

class DeviceManagerAdaptor;

namespace KS
{
class DeviceManager;

// 对外提供设备管理DBUS接口
class DeviceDBus : public QObject,
                   protected QDBusContext
{
    Q_OBJECT

public:
    DeviceDBus(DeviceManager *deviceManager, QObject *parent = nullptr);
    virtual ~DeviceDBus(){};

    void init();

public Q_SLOTS:  // METHODS
    // 获取所有设备信息
    QString GetDevices();

    // 获取指定接口类型的设备
    QString GetDevicesByInterface(int interfaceType);

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
    void EnableInterface(int type, bool enabled);

    // 获取连接记录
    QString GetRecords();

private:
    DeviceManager *m_deviceManager;
    DeviceManagerAdaptor *m_dbusAdaptor;
};

}  // namespace KS
