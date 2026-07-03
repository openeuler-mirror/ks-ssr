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
#include "src/daemon/device/sd-device-enumerator.h"
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

    connect(&m_devMonitor, &SdDeviceMonitor::udevSignal, this, &DeviceManager::handleUdevEvent);

    auto connection = QDBusConnection::systemBus();
    if (!connection.registerObject(KSC_DEVICE_MANAGER_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "Failed to register object:" << connection.lastError();
    }
}

void DeviceManager::initDevices()
{
    SdDeviceEnumerator enumerator;
    QList<QSharedPointer<SdDevice>> list = enumerator.getDevices();

    Q_FOREACH (auto device, list)
    {
        this->addDevice(device.data());
    }
}

void DeviceManager::addDevice(SdDevice *device)
{
    auto syspath = device->get_syspath();

    RETURN_IF_TRUE(syspath.isNull())

    auto dev = m_devFactory.createDevice(device);

    if (dev)
    {
        m_devMap.insert(dev->getSyspath(), dev);
        KLOG_INFO() << "Device added with syspath " << dev->getSyspath();
    }
}

QString DeviceManager::GetDevices()
{
    auto devices = m_devMap.values();
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;

    Q_FOREACH (QSharedPointer<Device> device, devices)
    {
        auto jsonObj = makeDevcieJsonInfo(device);

        jsonArray.append(jsonObj);
    }

    jsonDoc.setArray(jsonArray);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QString DeviceManager::GetDevice(const QString &id)
{
    auto devices = m_devMap.values();
    QJsonDocument jsonDoc;

    Q_FOREACH (QSharedPointer<Device> device, devices)
    {
        if (device->getId() == id)
        {
            auto jsonObj = makeDevcieJsonInfo(device);

            jsonDoc.setObject(jsonObj);

            break;
        }
    }

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QJsonObject DeviceManager::makeDevcieJsonInfo(QSharedPointer<Device> device)
{
    auto permission = device->getPermission();
    QJsonObject jsonObj{
        {KSC_DEVICE_KEY_ID, device->getId()},
        {KSC_DEVICE_KEY_NAME, device->getName()},
        {KSC_DEVICE_KEY_TYPE, device->getType()},
        {KSC_DEVICE_KEY_INTERFACE_TYPE, device->getInterfaceType()},
        {KSC_DEVICE_KEY_READ, permission->read},
        {KSC_DEVICE_KEY_WRITE, permission->write},
        {KSC_DEVICE_KEY_EXECUTE, permission->execute},
        {KSC_DEVICE_KEY_STATE, device->getState()}};

    return jsonObj;
}

bool DeviceManager::ChangePermission(const QString &permissions)
{
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
    auto id = GET_JSON_STRING_VALUE(jsonObj, KSC_DEVICE_KEY_ID);
    auto enable = GET_JSON_BOOL_VALUE(jsonObj, KSC_DEVICE_KEY_ENABLE);

    auto permission = QSharedPointer<Permission>(new Permission{
        .read = GET_JSON_BOOL_VALUE(jsonObj, KSC_DEVICE_KEY_READ),
        .write = GET_JSON_BOOL_VALUE(jsonObj, KSC_DEVICE_KEY_WRITE),
        .execute = GET_JSON_BOOL_VALUE(jsonObj, KSC_DEVICE_KEY_EXECUTE),
    });

#undef GET_JSON_STRING_VALUE
#undef GET_JSON_BOOL_VALUE

    if (id == nullptr)
    {
        KLOG_ERROR() << "QJsonDocument can not get id with " << permissions;
        return false;
    }

    auto device = this->findDevice(QString(id));

    if (device == nullptr)
    {
        KLOG_ERROR() << "Failed to find device with id " << id;
        return false;
    }

    device->setPermission(permission);

    auto ret = device->setEnable(enable);
    RETURN_VAL_IF_TRUE(ret < 0, false)

    // 重放该设备Udev事件
    device->trigger();

    return true;
}

void DeviceManager::handleUdevEvent(SdDevice *device)
{
    RETURN_IF_TRUE(device->get_syspath().isNull());
    auto action = device->get_action();

    if (action == SD_DEVICE_ACTION_REMOVE)
    {
        this->handleUdevRemoveEvent(device);
    }
    else if (action == SD_DEVICE_ACTION_ADD)
    {
        this->handleUdevAddEvent(device);
    }
    else if (action == SD_DEVICE_ACTION_CHANGE)
    {
        this->handleUdevChangeEvent(device);
    }
}

void DeviceManager::handleUdevAddEvent(SdDevice *device)
{
    auto syspath = device->get_syspath();

    RETURN_IF_TRUE(m_devMap.contains(syspath));

    this->addDevice(device);

    auto dev = m_devMap.value(syspath);
    if (dev)
    {
        Q_EMIT m_dbusAdaptor->DeviceChanged(dev->getId(), DEVICE_ACTION_ADD);
    }
}

void DeviceManager::handleUdevRemoveEvent(SdDevice *device)
{
    auto syspath = device->get_syspath();
    auto dev = m_devMap.take(syspath);

    if (dev)
    {
        KLOG_INFO() << "Device removed with syspath " << syspath;

        Q_EMIT m_dbusAdaptor->DeviceChanged(dev->getId(), DEVICE_ACTION_REMOVE);
    }
}

void DeviceManager::handleUdevChangeEvent(SdDevice *device)
{
    auto syspath = device->get_syspath();
    auto dev = m_devMap.value(syspath);

    if (dev)
    {
        KLOG_INFO() << "Device changed with syspath " << syspath;

        dev->update();

        Q_EMIT m_dbusAdaptor->DeviceChanged(dev->getId(), DEVICE_ACTION_CHANGE);
    }
}

QSharedPointer<Device> DeviceManager::findDevice(const QString &id)
{
    auto devices = m_devMap.values();

    Q_FOREACH (auto device, devices)
    {
        if (device->getId() == id)
        {
            return device;
        }
    }

    return nullptr;
}
}  // namespace KS
