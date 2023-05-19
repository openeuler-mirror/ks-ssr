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

#include "src/daemon/device/device-manager.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "src/daemon/device/sd/sd-device-enumerator.h"
#include "src/daemon/device_manager_adaptor.h"

namespace KS
{
DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
    m_dbusAdaptor = new DeviceManagerAdaptor(this);

    this->init();
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::init()
{
    this->initDevices();

    auto connection = QDBusConnection::systemBus();
    if (!connection.registerObject(KSC_DEVICE_MANAGER_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "Failed to register object:" << connection.lastError();
    }

    connect(&m_devMonitor, &SDDeviceMonitor::deviceChanged, this, &DeviceManager::handleUdevEvent);
}

void DeviceManager::initDevices()
{
    SDDeviceEnumerator enumerator;
    auto devices = enumerator.getDevices();

    Q_FOREACH (auto device, devices)
    {
        this->addDevice(device);
    }
}

void DeviceManager::addDevice(SDDevice *sdDevice)
{
    auto syspath = sdDevice->getSyspath();

    RETURN_IF_TRUE(syspath.isNull())

    auto device = m_devFactory.createDevice(sdDevice);

    if (device)
    {
        m_devices.insert(device->getSyspath(), device);
        KLOG_INFO() << "Device added with syspath " << device->getSyspath();
    }
}

QString
DeviceManager::GetDevices()
{
    auto devices = m_devices.values();
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;

    Q_FOREACH (auto device, devices)
    {
        jsonArray.append(device->toJsonObject());
    }

    jsonDoc.setArray(jsonArray);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QString DeviceManager::GetDevice(const QString &id)
{
    auto devices = m_devices.values();
    QJsonDocument jsonDoc;

    Q_FOREACH (auto device, devices)
    {
        if (device->getID() == id)
        {
            jsonDoc.setObject(device->toJsonObject());

            break;
        }
    }

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

bool DeviceManager::ChangePermission(const QString &id,
                                     const QString &permissions)
{
    auto device = this->findDevice(id);

    if (device == nullptr)
    {
        KLOG_ERROR() << "Failed to find device with id " << id;
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
    device->setEnable(true);

    // 重放该设备Udev事件
    device->trigger();

    return true;
}

bool DeviceManager::Enable(const QString &id)
{
    auto device = this->findDevice(id);

    if (device == nullptr)
    {
        KLOG_ERROR() << "Failed to find device with id " << id;
        return false;
    }

    device->setEnable(true);

    // 重放该设备Udev事件
    device->trigger();

    return true;
}

bool DeviceManager::Disable(const QString &id)
{
    auto device = this->findDevice(id);

    if (device == nullptr)
    {
        KLOG_ERROR() << "Failed to find device with id " << id;
        return false;
    }

    device->setEnable(false);

    // 重放该设备Udev事件
    device->trigger();

    return true;
}

QString DeviceManager::GetInterfaces()
{
    return m_devInterface.getInterfaces();
}

QString DeviceManager::GetInterface(int type)
{
    return m_devInterface.getInterface(type);
}

bool DeviceManager::EnableInterface(int type)
{
    return m_devInterface.setInterfaceEnable(type, true);
}

bool DeviceManager::DisableInterface(int type)
{
    return m_devInterface.setInterfaceEnable(type, false);
}

QString DeviceManager::GetRecords()
{
    auto records = m_devLog.getDeviceRecords();
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;

    Q_FOREACH (auto record, records)
    {
        jsonArray.append(m_devLog.toJsonObject(record));
    }

    jsonDoc.setArray(jsonArray);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

void DeviceManager::handleUdevEvent(SDDevice *device,
                                    int action)
{
    switch (action)
    {
    case SD_DEVICE_ACTION_REMOVE:
        this->handleUdevRemoveEvent(device);
        break;

    case SD_DEVICE_ACTION_ADD:
        this->handleUdevAddEvent(device);
        break;

    case SD_DEVICE_ACTION_CHANGE:
        this->handleUdevChangeEvent(device);
        break;

    default:
        break;
    }
}

void DeviceManager::recordDeviceConnection(QSharedPointer<Device> device)
{
    DeviceRecord record;

    record.name = device->getName();
    record.type = device->getType();

    record.state = device->getState() == DEVICE_STATE_ENABLE ? DEVICE_CONNECT_SUCCESSED : DEVICE_CONNECT_FAILED;

    // 以秒为单位的时间戳
    record.time = QDateTime::currentSecsSinceEpoch();

    m_devLog.addDeviceRecord(record);
}

void DeviceManager::handleUdevAddEvent(SDDevice *sdDevice)
{
    auto syspath = sdDevice->getSyspath();

    RETURN_IF_TRUE(m_devices.contains(syspath));

    this->addDevice(sdDevice);

    auto device = m_devices.value(syspath);
    if (device)
    {
        this->recordDeviceConnection(device);
        Q_EMIT m_dbusAdaptor->DeviceChanged(device->getID(), DEVICE_ACTION_ADD);
    }
}

void DeviceManager::handleUdevRemoveEvent(SDDevice *sdDevice)
{
    auto syspath = sdDevice->getSyspath();
    auto device = m_devices.take(syspath);

    if (device)
    {
        KLOG_INFO() << "Device removed with syspath " << syspath;
        Q_EMIT m_dbusAdaptor->DeviceChanged(device->getID(), DEVICE_ACTION_REMOVE);
    }
}

void DeviceManager::handleUdevChangeEvent(SDDevice *sdDevice)
{
    auto syspath = sdDevice->getSyspath();
    auto device = m_devices.value(syspath);

    if (device)
    {
        KLOG_INFO() << "Device changed with syspath " << syspath;
        Q_EMIT m_dbusAdaptor->DeviceChanged(device->getID(), DEVICE_ACTION_CHANGE);
    }
}

QSharedPointer<Device> DeviceManager::findDevice(const QString &id)
{
    auto devices = m_devices.values();

    Q_FOREACH (auto device, devices)
    {
        if (device->getID() == id)
        {
            return device;
        }
    }

    return nullptr;
}
}  // namespace KS
