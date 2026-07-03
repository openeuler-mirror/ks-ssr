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
#include "kss-dbus.h"

#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDBusError>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSharedPointer>
#include "config.h"
#include "include/ksc-error-i.h"
#include "include/ksc-i.h"
#include "include/ksc-marcos.h"
#include "lib/base/error.h"
#include "src/daemon/common/polkit-proxy.h"
#include "src/daemon/kss/kss-wrapper.h"
#include "src/daemon/kss_dbus_adaptor.h"

namespace KS
{
#define KSS_PERMISSION_AUTHENTICATION "com.kylinsec.SC.PermissionAuthentication"

// Box fount
#define RETURN_DBUS_ERROR_BOX_NOFOUND_IF_TRUE(cond, errorCode)                                               \
    {                                                                                                        \
        if (cond)                                                                                            \
        {                                                                                                    \
            auto replyMessage = message.createErrorReply(QDBusError::InvalidArgs, KSC_ERROR2STR(errorCode)); \
            QDBusConnection::systemBus().send(replyMessage);                                                 \
            return;                                                                                          \
        }                                                                                                    \
    }

KSSDbus::KSSDbus(QObject *parent) : QObject(parent)
{
    m_dbusAdaptor = new KSSDbusAdaptor(this);

    init();

    connect(KSSWrapper::getDefault().get(), SIGNAL(initFinished()), this, SIGNAL(InitFinished()));
}

int KSSDbus::initialized() const
{
    return KSSWrapper::getDefault()->getInitialized();
}

CHECK_AUTH_WITH_1ARGS(KSSDbus, AddTPFile, addTPFileAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, RemoveTPFile, removeTPFileAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_2ARGS(KSSDbus, ProhibitUnloading, prohibitUnloadingAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, bool, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, AddFPFile, addFPFileAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, RemoveFPFile, removeFPFileAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QString &);

QString KSSDbus::GetExecuteFiles()
{
    return KSSWrapper::getDefault()->getExecuteFiles();
}

QString KSSDbus::GetFPFiles()
{
    return KSSWrapper::getDefault()->getFiles();
}

QString KSSDbus::GetModuleFiles()
{
    return KSSWrapper::getDefault()->getModuleFiles();
}

QString KSSDbus::Search(const QString &pathKey, uint searchType)
{
    RETURN_VAL_DBUS_ERROR_IF_TRUE(pathKey.isEmpty(), QString())

    RETURN_VAL_IF_TRUE(KSSType(searchType) == KSSType::KSS_TYPE_NONE, QString())
    QString fileList;
    QJsonDocument resultJsonDoc;
    QJsonArray jsonArr;
    QJsonParseError jsonError;

    if (KSSType(searchType) == KSSType::KSS_TYPE_TP_EXECUTE)
    {
        fileList = KSSWrapper::getDefault()->getExecuteFiles();
    }
    else if (KSSType(searchType) == KSSType::KSS_TYPE_TP_KERNEL)
    {
        fileList = KSSWrapper::getDefault()->getModuleFiles();
    }
    else if (KSSType(searchType) == KSSType::KSS_TYPE_FP)
    {
        fileList = KSSWrapper::getDefault()->getFiles();
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

void KSSDbus::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(KSC_KSS_INIT_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}

void KSSDbus::addTPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    RETURN_DBUS_ERROR_BOX_NOFOUND_IF_TRUE(filePath.isEmpty(), KSCErrorCode::ERROR_COMMON_INVALID_ARGS)

    auto output = KSSWrapper::getDefault()->addTrustedFile(filePath);
    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(output.toUtf8(), &jsonError);

    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        return;
    }

    if (jsonDoc.object().value(KSC_KSS_JK_COUNT).toInt() == 0)
    {
        auto replyMessage = message.createErrorReply(QDBusError::Failed, KSC_ERROR2STR(KSCErrorCode::ERROR_TP_ADD_INVALID_FILE));
        QDBusConnection::systemBus().send(replyMessage);
        return;
    }

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::removeTPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    RETURN_DBUS_ERROR_BOX_NOFOUND_IF_TRUE(filePath.isEmpty(), KSCErrorCode::ERROR_COMMON_INVALID_ARGS)

    KSSWrapper::getDefault()->removeTrustedFile(filePath);

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::prohibitUnloadingAfterAuthorization(const QDBusMessage &message, bool prohibited, const QString &filePath)
{
    RETURN_DBUS_ERROR_BOX_NOFOUND_IF_TRUE(filePath.isEmpty(), KSCErrorCode::ERROR_COMMON_INVALID_ARGS)

    KSSWrapper::getDefault()->prohibitUnloading(prohibited, filePath);

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::addFPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    RETURN_DBUS_ERROR_BOX_NOFOUND_IF_TRUE(filePath.isEmpty(), KSCErrorCode::ERROR_COMMON_INVALID_ARGS)

    QFileInfo fileInfo(filePath);
    auto fileName = fileInfo.fileName();
    KSSWrapper::getDefault()->addFile(fileName, filePath, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::removeFPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    RETURN_DBUS_ERROR_BOX_NOFOUND_IF_TRUE(filePath.isEmpty(), KSCErrorCode::ERROR_COMMON_INVALID_ARGS)

    KSSWrapper::getDefault()->removeFile(filePath);

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

}  // namespace KS
