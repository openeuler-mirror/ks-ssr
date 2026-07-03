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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "config.h"
#include "include/ksc-i.h"
#include "include/ksc-marcos.h"
#include "src/daemon/trusted_adaptor.h"

namespace KS
{
Trusted::Trusted(QObject *parent) : QObject(parent)
{
    m_dbusAdaptor = new TrustedAdaptor(this);
    m_kss = new KSS(this);
    connect(m_kss, &KSS::initFinished, this, &Trusted::InitFinished);

    init();
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
    RETURN_VAL_IF_TRUE(TrustedProtectType(searchType) == TrustedProtectType::TRUSTED_PROTECT_TYPE_NONE, QString())
    QString fileList;
    QJsonDocument resultJsonDoc;
    QJsonArray jsonArr;
    QJsonParseError jsonError;

    if (TrustedProtectType(searchType) == TrustedProtectType::TRUSTED_PROTECT_TYPE_KERNEL)
    {
        fileList = m_kss->getModuleFiles();
    }
    else
    {
        fileList = m_kss->getExecuteFiles();
    }

    auto jsonDoc = QJsonDocument::fromJson(fileList.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        return QString();
    }

    auto jsonModules = jsonDoc.object().value(KSC_JK_DATA).toArray();
    for (const auto &module : jsonModules)
    {
        auto jsonMod = module.toObject();

        // 通过输入的pathKey，判断list中path字段是否包含pathKey
        if (jsonMod.value(KSC_JK_DATA_PATH).toString().contains(pathKey))
        {
            jsonArr.push_back(jsonMod);
        }
    }
    resultJsonDoc.setArray(jsonArr);
    return QString(resultJsonDoc.toJson());
}

void Trusted::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(KSC_TRUSTED_PROTECTED_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}

}  // namespace KS
