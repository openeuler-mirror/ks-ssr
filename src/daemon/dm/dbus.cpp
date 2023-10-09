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

#include <qt5-log-i.h>
#include <ssr-error-i.h>
#include <ssr-i.h>
#include <ssr-marcos.h>
#include "lib/base/error.h"
#include "src/daemon/common/polkit-proxy.h"
#include "src/daemon/device_manager_adaptor.h"
#include "src/daemon/dm/configuration.h"
#include "src/daemon/dm/dbus.h"
#include "src/daemon/dm/device-manager.h"

namespace KS
{
namespace DM
{
DBus::DBus(DeviceManager *deviceManager, QObject *parent) : QObject(parent),
                                                            m_deviceManager(deviceManager)
{
    m_dbusAdaptor = new DeviceManagerAdaptor(this);
}

void DBus::init()
{
    auto connection = QDBusConnection::systemBus();
    if (!connection.registerObject(SSR_DEVICE_MANAGER_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Failed to register object:" << connection.lastError();
    }

    connect(m_deviceManager, SIGNAL(deviceChanged(const QString &, int)), m_dbusAdaptor, SIGNAL(DeviceChanged(const QString &, int)));
}

CHECK_AUTH_WITH_2ARGS(DBus, ChangePermission, changePermission, SSR_PERMISSION_AUTHENTICATION, const QString &, const QString &)
CHECK_AUTH_WITH_1ARGS(DBus, Enable, enable, SSR_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(DBus, Disable, disable, SSR_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_2ARGS(DBus, EnableInterface, enableInterface, SSR_PERMISSION_AUTHENTICATION, int, bool)

QString DBus::GetDevices()
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

QString DBus::GetDevicesByInterface(int interfaceType)
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;

    if (interfaceType <= INTERFACE_TYPE_UNKNOWN || interfaceType >= INTERFACE_TYPE_LAST)
    {
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_DEVICE_INVALID_IFC_TYPE, this->message())
    }

    auto devices = m_deviceManager->getDevicesByInterface(interfaceType);
    for (auto device : devices)
    {
        jsonArray.append(device->toJsonObject());
    }

    jsonDoc.setArray(jsonArray);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QString DBus::GetDevice(const QString &id)
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
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_DEVICE_INVALID_ID, this->message())
    }

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QString DBus::GetInterfaces()
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;

    auto deviceConfiguration = Configuration::instance();
    for (int type = INTERFACE_TYPE_USB; type < INTERFACE_TYPE_LAST; ++type)
    {
#ifndef _345_GC_
        if (type == INTERFACE_TYPE_HDMI)
        {
            continue;
        }
#endif
        QJsonObject jsonObj{
            {SSR_DI_JK_TYPE, type},
            {SSR_DI_JK_ENABLE, deviceConfiguration->isIFCEnable(type)}};

        jsonArray.append(jsonObj);
    }

    jsonDoc.setArray(jsonArray);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QString DBus::GetInterface(int type)
{
    if (type <= INTERFACE_TYPE_UNKNOWN || type >= INTERFACE_TYPE_LAST)
    {
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_DEVICE_INVALID_IFC_TYPE, this->message())
    }

    QJsonDocument jsonDoc;
    auto deviceConfiguration = Configuration::instance();

    QJsonObject jsonObj{
        {SSR_DI_JK_TYPE, type},
        {SSR_DI_JK_ENABLE, deviceConfiguration->isIFCEnable(type)}};
    jsonDoc.setObject(jsonObj);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

void DBus::changePermission(const QDBusMessage &message,
                            const QString &id,
                            const QString &permissions)
{
    auto device = m_deviceManager->getDeviceByID(id);

    if (!device)
    {
        KLOG_WARNING() << "Failed to find device with id " << id;
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_DEVICE_INVALID_ID, message)
    }

    QJsonParseError error;
    auto jsonDoc = QJsonDocument::fromJson(permissions.toLocal8Bit(), &error);

    if (error.error != QJsonParseError::NoError)
    {
        KLOG_WARNING() << "Failed to create QJsonDocument with " << permissions;
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_DEVICE_INVALID_PERM, message)
    }

    if (!jsonDoc.isObject())
    {
        KLOG_WARNING() << "QJsonDocument is not object with " << permissions;
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_DEVICE_INVALID_PERM, message)
    }

#define GET_JSON_STRING_VALUE(obj, key) ((obj).value(key).isString() ? (obj).value(key).toString() : nullptr)
#define GET_JSON_BOOL_VALUE(obj, key) ((obj).value(key).isBool() ? (obj).value(key).toBool() : false)

    auto jsonObj = jsonDoc.object();
    auto permission = QSharedPointer<Permission>(new Permission{
        .read = GET_JSON_BOOL_VALUE(jsonObj, SSR_DEVICE_JK_READ),
        .write = GET_JSON_BOOL_VALUE(jsonObj, SSR_DEVICE_JK_WRITE),
        .execute = GET_JSON_BOOL_VALUE(jsonObj, SSR_DEVICE_JK_EXECUTE),
    });

#undef GET_JSON_STRING_VALUE
#undef GET_JSON_BOOL_VALUE
    device->setPermission(permission);
    m_deviceManager->checkDeviceMount(device);

    device->setEnable(true);

    // 重放该设备Udev事件
    device->trigger();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void DBus::enable(const QDBusMessage &message,
                  const QString &id)
{
    auto device = m_deviceManager->getDeviceByID(id);

    if (!device)
    {
        KLOG_WARNING() << "Failed to find device with id " << id;
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_DEVICE_INVALID_ID, message)
    }

    device->setEnable(true);
    // 重放该设备Udev事件
    device->trigger();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void DBus::disable(const QDBusMessage &message,
                   const QString &id)
{
    auto device = m_deviceManager->getDeviceByID(id);

    if (!device)
    {
        KLOG_WARNING() << "Failed to find device with id " << id;
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_DEVICE_INVALID_ID, message)
    }

    device->setEnable(false);
    // 重放该设备Udev事件
    device->trigger();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void DBus::enableInterface(const QDBusMessage &message,
                           int type,
                           bool enabled)
{
    if (type <= INTERFACE_TYPE_UNKNOWN || type >= INTERFACE_TYPE_LAST)
    {
        KLOG_WARNING() << "Illegal interface type " << type;
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_DEVICE_INVALID_IFC_TYPE, message)
    }

    if ((type == INTERFACE_TYPE_HDMI) && !m_deviceManager->isSupportHDMIDisable())
    {
        KLOG_WARNING() << "Not support disable HDMI interface.";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_DEVICE_DISABLE_HDMI, message)
    }

    Configuration::instance()->setIFCEnable(type, enabled);
    m_deviceManager->triggerInterfaceDevices(type);

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

QString DBus::GetRecords()
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
}  // namespace DM
}  // namespace KS
