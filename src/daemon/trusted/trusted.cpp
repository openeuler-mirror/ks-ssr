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

#include "src/daemon/trusted/trusted.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include "config.h"
#include "include/sc-i.h"
#include "include/sc-marcos.h"
#include "src/daemon/trusted_adaptor.h"

namespace KS
{
Trusted::Trusted(QObject *parent) : QObject(parent)
{
    this->m_dbusAdaptor = new TrustedAdaptor(this);
    m_kss = new Kss(this);
    connect(m_kss, &Kss::initFinished, this, &Trusted::InitFinished);

    this->init();
}
Trusted::~Trusted()
{
}

void Trusted::AddFile(const QString &filePath)
{
    m_kss->addTrustedFile(filePath);
}

QString Trusted::GetExecuteFiles()
{
    return m_kss->getExecuteFiles();
}

QString Trusted::GetModuleFiles()
{
    return m_kss->getModuleFiles();
}

void Trusted::ProhibitUnloading(bool prohibited, const QString &filePath)
{
}

void Trusted::RemoveFile(const QString &filePath)
{
    m_kss->removeTrustedFile(filePath);
}

QString Trusted::Search(const QString &pathKey, uint searchType)
{
    RETURN_VAL_IF_TRUE(TRUSTED_PROTECT_TYPE(searchType) == TRUSTED_PROTECT_TYPE::OTHER_TRUSTED_PROTECT, QString())

    if (TRUSTED_PROTECT_TYPE(searchType) == TRUSTED_PROTECT_TYPE::KERNEL_PROTECT)
    {
        return m_kss->search(pathKey, m_kss->getModuleFiles());
    }
    else
    {
        return m_kss->search(pathKey, m_kss->getExecuteFiles());
    }
}

void Trusted::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(SC_TRUSTED_PROTECTED_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}

}  // namespace KS
