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
#include "license-manager.h"
#include <QDBusConnection>
#include <QDateTime>
#include "include/ksc-i.h"
#include "include/ksc-marcos.h"
#include "src/daemon/license/license-proxy.h"
#include "src/daemon/license_manager_adaptor.h"

namespace KS
{
LicenseManager *LicenseManager::m_instance = nullptr;
void LicenseManager::globalInit(QObject *parent)
{
    m_instance = new LicenseManager(parent);
}

void LicenseManager::globalDeinit()
{
    if (m_instance)
    {
        delete m_instance;
    }
}

LicenseManager::LicenseManager(QObject *parent) : QObject(parent)
{
    m_dbusAdaptor = new LicenseAdaptor(this);
    m_licenseProxy = LicenseProxy::getDefault();
    connect(m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, &LicenseManager::LicenseChanged);
    init();
}

void LicenseManager::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(KSC_LICENSE_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}

bool LicenseManager::activated() const
{
    return m_licenseProxy->isActivated();
}

void LicenseManager::ActivateByActivationCode(const QString &activeCode)
{
    QString errorMessage;
    if (!m_licenseProxy->activateByActivationCode(activeCode, errorMessage))
    {
        sendErrorReply(QDBusError::Failed, errorMessage);
        return;
    }
}

QString LicenseManager::GetActivationCode()
{
    return m_licenseProxy->getActivationCode();
}

QString LicenseManager::GetExpiredTime()
{
    return QDateTime::fromSecsSinceEpoch(m_licenseProxy->getExpiredTime()).toString("yyyy-MM-dd");
}

QString LicenseManager::GetMachineCode()
{
    return m_licenseProxy->getMachineCode();
}
}  // namespace KS
