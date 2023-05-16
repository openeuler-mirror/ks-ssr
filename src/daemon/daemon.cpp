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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#include "src/daemon/daemon.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include "include/ksc-i.h"
#include "src/daemon/box/box-manager.h"
#include "src/daemon/device/device-manager.h"
#include "src/daemon/fp/fp.h"
#include "src/daemon/tp/tp.h"

namespace KS
{
Daemon *Daemon::m_instance = nullptr;

Daemon::Daemon() : QObject(nullptr)
{
    m_boxManager = new BoxManager(this);
    m_trusted = new TP(this);
    m_fileProtected = new FP(this);
    m_deviceManger = new DeviceManager(this);
}

Daemon::~Daemon()
{
}

void Daemon::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerService(KSC_DBUS_NAME))
    {
        KLOG_WARNING() << "Failed to register dbus name: " << KSC_DBUS_NAME;
    }

    if (!connection.registerObject(KSC_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}
}  // namespace KS
