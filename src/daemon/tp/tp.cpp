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

#include "src/daemon/tp/tp.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "config.h"
#include "include/ksc-i.h"
#include "include/ksc-marcos.h"
#include "include/ksc-error-i.h"
#include "lib/base/error.h"
#include "src/daemon/common/kss.h"
#include "src/daemon/tp_adaptor.h"

namespace KS
{
TP::TP(QObject *parent) : QObject(parent)
{
    m_dbusAdaptor = new TPAdaptor(this);
    connect(KSS::getDefault().get(), SIGNAL(initFinished()), this, SIGNAL(InitFinished()));

    init();
}
TP::~TP()
{
}

void TP::AddFile(const QString &filePath)
{
    if (filePath.isEmpty())
    {
        sendErrorReply(QDBusError::InvalidArgs, KSC_ERROR2STR(KSCErrorCode::ERROR_COMMON_INVALID_ARGS));
        return ;
    }

    auto output = KSS::getDefault()->addTrustedFile(filePath);
    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(output.toUtf8(), &jsonError);

    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        return;
    }

    if (jsonDoc.object().value(KSC_KSS_JK_COUNT).toInt() == 0)
    {
        sendErrorReply(QDBusError::Failed, KSC_ERROR2STR(KSCErrorCode::ERROR_TP_ADD_INVALID_FILE));
        return;
    }
}

QString TP::GetExecuteFiles()
{
    return KSS::getDefault()->getExecuteFiles();
}

QString TP::GetModuleFiles()
{
    return KSS::getDefault()->getModuleFiles();
}

void TP::ProhibitUnloading(bool prohibited, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        sendErrorReply(QDBusError::InvalidArgs, KSC_ERROR2STR(KSCErrorCode::ERROR_COMMON_INVALID_ARGS));
        return ;
    }

    KSS::getDefault()->prohibitUnloading(prohibited, filePath);
}

void TP::RemoveFile(const QString &filePath)
{
    if (filePath.isEmpty())
    {
        sendErrorReply(QDBusError::InvalidArgs, KSC_ERROR2STR(KSCErrorCode::ERROR_COMMON_INVALID_ARGS));
        return ;
    }

    KSS::getDefault()->removeTrustedFile(filePath);
}

QString TP::Search(const QString &pathKey, uint searchType)
{
    if (pathKey.isEmpty())
    {
        sendErrorReply(QDBusError::InvalidArgs, KSC_ERROR2STR(KSCErrorCode::ERROR_COMMON_INVALID_ARGS));
        return QString();
    }

    RETURN_VAL_IF_TRUE(TrustedProtectType(searchType) == TrustedProtectType::TRUSTED_PROTECT_TYPE_NONE, QString())
    QString fileList;
    QJsonDocument resultJsonDoc;
    QJsonArray jsonArr;
    QJsonParseError jsonError;

    if (TrustedProtectType(searchType) == TrustedProtectType::TRUSTED_PROTECT_TYPE_KERNEL)
    {
        fileList = KSS::getDefault()->getModuleFiles();
    }
    else
    {
        fileList = KSS::getDefault()->getExecuteFiles();
    }

    auto jsonDoc = QJsonDocument::fromJson(fileList.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        return QString();
    }

    auto jsonModules = jsonDoc.object().value(KSC_KSS_JK_DATA).toArray();
    for (const auto &module : jsonModules)
    {
        auto jsonMod = module.toObject();

        // 通过输入的pathKey，判断list中path字段是否包含pathKey
        if (jsonMod.value(KSC_KSS_JK_DATA_PATH).toString().contains(pathKey))
        {
            jsonArr.push_back(jsonMod);
        }
    }
    resultJsonDoc.setArray(jsonArr);
    return QString(resultJsonDoc.toJson());
}

void TP::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(KSC_TP_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}

}  // namespace KS
