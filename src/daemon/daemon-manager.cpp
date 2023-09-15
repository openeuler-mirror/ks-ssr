/**
 * @file          /ks-sc/src/daemon/daemon-manager.cpp
 * @brief         
 * @author        chendingjian <chendingjian@kylinos.com>
 * @copyright (c) 2023 KylinSec. All rights reserved.
 */
#include "src/daemon/daemon-manager.h"
#include "src/daemon/box/box-dao.h"
#include <QDBusConnection>

namespace KS
{
DaemonManager *DaemonManager::m_instance = nullptr;

DaemonManager::DaemonManager() : QObject(nullptr)
{
    BoxDao::globalInit();
    boxManger = new BoxManager(this);
}

DaemonManager::~DaemonManager()
{
}

void DaemonManager::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerService(SC_DBUS_NAME))
    {
        KLOG_WARNING() << "Failed to register dbus name: " << SC_DBUS_NAME;
    }

    if (!connection.registerObject(SC_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}
}  // namespace KS
