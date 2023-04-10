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

#include "src/daemon/protected/file-protected.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDateTime>
#include "config.h"
#include "include/sc-i.h"
#include "include/sc-marcos.h"
#include "src/daemon/file_protected_adaptor.h"

namespace KS
{
FileProtected::FileProtected(QObject *parent) : QObject(parent)
{
    this->m_dbusAdaptor = new FileProtectedAdaptor(this);
    m_kss = new Kss(this);

    this->init();
}
FileProtected::~FileProtected()
{
}

void FileProtected::AddFile(const QString &filePath)
{
    auto fileName = filePath.section('/', -1);
//    KLOG_DEBUG() << "Add file name is " << fileName;
    m_kss->addFile(fileName, filePath, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

QString FileProtected::GetFiles()
{
    return m_kss->getFiles();
}

void FileProtected::RemoveFile(const QString &filePath)
{
    m_kss->removeFile(filePath);
}

QString FileProtected::Search(const QString &pathKey)
{
    return m_kss->search(pathKey, m_kss->getFiles());
}

void FileProtected::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(SC_FILE_PROTECTED_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}

}  // namespace KS
