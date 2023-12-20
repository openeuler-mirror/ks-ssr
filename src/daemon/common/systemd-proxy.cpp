/**
 * Copyright (c) 2022 ~ 2023 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/daemon/common/systemd-proxy.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

namespace KS
{
#define SYSTEMD_DBUS_NAME "org.freedesktop.systemd1"
#define SYSTEMD_DBUS_OBJECT_PATH "/org/freedesktop/systemd1"
#define SYSTEMD_DBUS_INTERFACE_NAME "org.freedesktop.systemd1.Manager"

SystemdProxy::SystemdProxy()
{
}

QSharedPointer<SystemdProxy> SystemdProxy::m_instance = nullptr;
QSharedPointer<SystemdProxy> SystemdProxy::getDefault()
{
    if (!m_instance)
    {
        m_instance = QSharedPointer<SystemdProxy>::create();
    }
    return m_instance;
}

void SystemdProxy::stopAndDisableUnit(const QString &name)
{
    this->stopUnit(name);
    this->disableUnit(name);
}

void SystemdProxy::disableUnit(const QString &name)
{
    auto arguments = QList<QVariant>{QStringList(name), false};
    this->callNoReplay("DisableUnitFiles", arguments);
    this->reload();
}

void SystemdProxy::stopUnit(const QString &name)
{
    auto arguments = QList<QVariant>{name, QString("replace")};
    this->callNoReplay("StopUnit", arguments);
}

void SystemdProxy::startAndEnableUnit(const QString &name)
{
    this->startUnit(name);
    this->enableUnit(name);
}

void SystemdProxy::enableUnit(const QString &name)
{
    auto arguments = QList<QVariant>{QStringList(name), false};
    this->callNoReplay("EnableUnitFiles", arguments);
    this->reload();
}

void SystemdProxy::startUnit(const QString &name)
{
    auto arguments = QList<QVariant>{name, QString("replace")};
    this->callNoReplay("StartUnit", arguments);
}

void SystemdProxy::reload()
{
    this->callNoReplay("Reload", QList<QVariant>());
}

void SystemdProxy::callNoReplay(const QString &method, const QList<QVariant> &arguments)
{
    auto sendMessage = QDBusMessage::createMethodCall(SYSTEMD_DBUS_NAME,
                                                      SYSTEMD_DBUS_OBJECT_PATH,
                                                      SYSTEMD_DBUS_INTERFACE_NAME,
                                                      method);

    sendMessage.setArguments(arguments);
    QDBusConnection::systemBus().asyncCall(sendMessage);
}
}  // namespace KS
