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

#include "src/daemon/device/device-dbus.h"
#include <ksc-i.h>
#include <qt5-log-i.h>
#include "src/daemon/device/device-manager.h"
#include "src/daemon/device/device-rule-manager.h"
#include "src/daemon/device_manager_adaptor.h"

namespace KS
{
DeviceDBus::DeviceDBus(DeviceManager *deviceManager, QObject *parent) : QObject(parent),
                                                                        m_deviceManager(deviceManager)
{
    m_dbusAdaptor = new DeviceManagerAdaptor(this);
}

void DeviceDBus::init()
{
    auto connection = QDBusConnection::systemBus();
    if (!connection.registerObject(KSC_DEVICE_MANAGER_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Failed to register object:" << connection.lastError();
    }

    connect(m_deviceManager, SIGNAL(deviceChanged(const QString &, int)), m_dbusAdaptor, SIGNAL(DeviceChanged(const QString &, int)));
}

QString DeviceDBus::GetDevices()
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;

    auto devices = m_deviceManager->getDevices();
    for (auto device : devices)
    {
        jsonArray.append(device->toJsonObject());
    }

    jsonDoc.setArray(jsonArray);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QString DeviceDBus::GetDevicesByInterface(int interfaceType)
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;

    auto devices = m_deviceManager->getDevicesByInterface(interfaceType);
    for (auto device : devices)
    {
        jsonArray.append(device->toJsonObject());
    }

    jsonDoc.setArray(jsonArray);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QString DeviceDBus::GetDevice(const QString &id)
{
    QJsonDocument jsonDoc;

    auto device = m_deviceManager->getDeviceByID(id);
    if (device)
    {
        jsonDoc.setObject(device->toJsonObject());
    }
    else
    {
        KLOG_WARNING() << "Not found device which id is  " << id;
    }

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QString DeviceDBus::GetInterfaces()
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;

    auto deviceRuleManager = DeviceRuleManager::instance();
    for (int type = INTERFACE_TYPE_USB; type < INTERFACE_TYPE_LAST; ++type)
    {
        QJsonObject jsonObj{
            {KSC_DI_JK_TYPE, type},
            {KSC_DI_JK_ENABLE, deviceRuleManager->isIFCEnable(type)}};

        jsonArray.append(jsonObj);
    }

    jsonDoc.setArray(jsonArray);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QString DeviceDBus::GetInterface(int type)
{
    // TODO: 判断type的合法性

    QJsonDocument jsonDoc;
    auto deviceRuleManager = DeviceRuleManager::instance();

    QJsonObject jsonObj{
        {KSC_DI_JK_TYPE, type},
        {KSC_DI_JK_ENABLE, deviceRuleManager->isIFCEnable(type)}};
    jsonDoc.setObject(jsonObj);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

bool DeviceDBus::ChangePermission(const QString &id,
                                  const QString &permissions)
{
    auto device = m_deviceManager->getDeviceByID(id);

    if (!device)
    {
        KLOG_WARNING() << "Failed to find device with id " << id;
        return false;
    }

    QJsonParseError error;
    auto jsonDoc = QJsonDocument::fromJson(permissions.toLocal8Bit(), &error);

    if (error.error != QJsonParseError::NoError)
    {
        KLOG_ERROR() << "Failed to create QJsonDocument with " << permissions;
        return false;
    }

    if (!jsonDoc.isObject())
    {
        KLOG_ERROR() << "QJsonDocument is not object with " << permissions;
        return false;
    }

#define GET_JSON_STRING_VALUE(obj, key) ((obj).value(key).isString() ? (obj).value(key).toString() : nullptr)
#define GET_JSON_BOOL_VALUE(obj, key) ((obj).value(key).isBool() ? (obj).value(key).toBool() : false)

    auto jsonObj = jsonDoc.object();
    auto permission = QSharedPointer<Permission>(new Permission{
        .read = GET_JSON_BOOL_VALUE(jsonObj, KSC_DEVICE_JK_READ),
        .write = GET_JSON_BOOL_VALUE(jsonObj, KSC_DEVICE_JK_WRITE),
        .execute = GET_JSON_BOOL_VALUE(jsonObj, KSC_DEVICE_JK_EXECUTE),
    });

#undef GET_JSON_STRING_VALUE
#undef GET_JSON_BOOL_VALUE
    device->setPermission(permission);
    m_deviceManager->checkDeviceMount(device);

    device->setEnable(true);

    // 重放该设备Udev事件
    device->trigger();

    return true;
}

bool DeviceDBus::Enable(const QString &id)
{
    auto device = m_deviceManager->getDeviceByID(id);

    // FIXME: 这个函数应该是没有返回值的，应该要返回错误信息才对，其他DBUS接口也一样
    if (!device)
    {
        KLOG_WARNING() << "Failed to find device with id " << id;
        return false;
    }

    device->setEnable(true);
    // 重放该设备Udev事件
    device->trigger();
    return true;
}

bool DeviceDBus::Disable(const QString &id)
{
    auto device = m_deviceManager->getDeviceByID(id);

    if (!device)
    {
        KLOG_ERROR() << "Failed to find device with id " << id;
        return false;
    }

    device->setEnable(false);
    // 重放该设备Udev事件
    device->trigger();
    return true;
}

void DeviceDBus::EnableInterface(int type, bool enabled)
{
    DeviceRuleManager::instance()->setIFCEnable(type, enabled);

    auto devices = m_deviceManager->getDevices();
    for (auto device : devices)
    {
        //重放此接口类型的设备的Udev事件
        if (device->getInterfaceType() == type)
        {
            device->trigger();
        }
    }
}

QString DeviceDBus::GetRecords()
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;

    auto deviceLog = m_deviceManager->getDeviceLog();
    auto records = deviceLog->getDeviceRecords();
    Q_FOREACH (auto record, records)
    {
        jsonArray.append(deviceLog->toJsonObject(record));
    }

    jsonDoc.setArray(jsonArray);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

}  // namespace KS
