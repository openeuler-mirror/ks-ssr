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
#include <QDateTime>
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

#define TRUSTED_STATUS_INITIALT_VALUE 0
#define TRUSTED_STATUS_OPEN 1
#define TRUSTED_STATUS_CLOSE 2

KSSDbus::KSSDbus(QObject *parent) : QObject(parent)
{
    m_dbusAdaptor = new KSSDbusAdaptor(this);

    init();

    connect(KSSWrapper::getDefault().get(), SIGNAL(initFinished()), this, SIGNAL(InitFinished()));
}

bool KSSDbus::initialized() const
{
    RETURN_VAL_IF_TRUE(KSSWrapper::getDefault()->getInitialized() == 0, false)

    return true;
}

uint KSSDbus::storageMode() const
{
    return KSSWrapper::getDefault()->getCurrentStorageMode();
}

bool KSSDbus::trustedStatus() const
{
    QJsonParseError jsonError;
    auto trustedStatus = KSSWrapper::getDefault()->getTrustedStatus();
    auto jsonDoc = QJsonDocument::fromJson(trustedStatus.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        return false;
    }

    auto status = jsonDoc.object().value("sm").toInt();
    RETURN_VAL_IF_TRUE(status == TRUSTED_STATUS_INITIALT_VALUE || status == TRUSTED_STATUS_CLOSE, false)

    return true;
}

CHECK_AUTH_WITH_1ARGS(KSSDbus, AddTrustedFile, addTPFileAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, AddTrustedFiles, addTPFilesAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QStringList &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, RemoveTrustedFile, removeTPFileAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, RemoveTrustedFiles, removeTPFilesAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QStringList &)
CHECK_AUTH_WITH_2ARGS(KSSDbus, ProhibitUnloading, prohibitUnloadingAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, bool, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, AddProtectedFile, addFPFileAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, AddProtectedFiles, addFPFilesAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QStringList &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, RemoveProtectedFile, removeFPFileAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, RemoveProtectedFiles, removeFPFilesAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, const QStringList &)
CHECK_AUTH_WITH_2ARGS(KSSDbus, SetStorageMode, setStorageModeAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, uint, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, SetTrustedStatus, setTrustedStatusAfterAuthorization, KSS_PERMISSION_AUTHENTICATION, bool);

QString KSSDbus::GetTrustedFiles(uint type)
{
    return KSSWrapper::getDefault()->getTrustedFiles(KSCKSSTrustedFileType(type));
}

QString KSSDbus::GetProtectedFiles()
{
    return KSSWrapper::getDefault()->getFiles();
}

QString KSSDbus::Search(const QString &pathKey, uint searchType)
{
    if (pathKey.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(),
                                        KSCErrorCode::ERROR_COMMON_INVALID_ARGS,
                                        message())
    }

    RETURN_VAL_IF_TRUE(KSSType(searchType) == KSSType::KSS_TYPE_NONE, QString())
    QString fileList;
    QJsonDocument resultJsonDoc;
    QJsonArray jsonArr;
    QJsonParseError jsonError;

    if (KSSType(searchType) == KSSType::KSS_TYPE_TP_EXECUTE)
    {
        fileList = KSSWrapper::getDefault()->getTrustedFiles(KSCKSSTrustedFileType::KSC_KSS_TRUSTED_FILE_TYPE_EXECUTE);
    }
    else if (KSSType(searchType) == KSSType::KSS_TYPE_TP_KERNEL)
    {
        fileList = KSSWrapper::getDefault()->getTrustedFiles(KSCKSSTrustedFileType::KSC_KSS_TRUSTED_FILE_TYPE_KERNEL);
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
    if (filePath.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }

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
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_TP_ADD_INVALID_FILE, message)
    }
    emit TrustedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::addTPFilesAfterAuthorization(const QDBusMessage &message, const QStringList &fileList)
{
    if (fileList.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }
    // TODO：暂时使用循环调用kss添加，kss有添加文件列表接口后修改为该接口
    for (auto filePath : fileList)
    {
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
            DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_TP_ADD_INVALID_FILE, message)
        }
    }

    emit TrustedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::removeTPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }

    KSSWrapper::getDefault()->removeTrustedFile(filePath);
    emit TrustedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::removeTPFilesAfterAuthorization(const QDBusMessage &message, const QStringList &fileList)
{
    if (fileList.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }
    // TODO：暂时使用循环调用kss移除，kss有添加文件列表接口后修改为该接口
    for (auto filePath : fileList)
    {
        KSSWrapper::getDefault()->removeTrustedFile(filePath);
    }

    emit TrustedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::prohibitUnloadingAfterAuthorization(const QDBusMessage &message, bool prohibited, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }

    KSSWrapper::getDefault()->prohibitUnloading(prohibited, filePath);
    emit TrustedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::addFPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }

    // 检测列表中是否存在相同文件
    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(KSSWrapper::getDefault()->getFiles().toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
    }

    auto jsonModules = jsonDoc.object().value(KSC_KSS_JK_DATA).toArray();
    for (const auto &module : jsonModules)
    {
        auto jsonMod = module.toObject();
        if (jsonMod.value(KSC_KSS_JK_DATA_PATH).toString() == filePath)
        {
            DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_TP_ADD_RECUR_FILE, message)
        }
    }
    // 添加文件
    QFileInfo fileInfo(filePath);
    auto fileName = fileInfo.fileName();
    KSSWrapper::getDefault()->addFile(fileName, filePath, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    emit ProtectedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::addFPFilesAfterAuthorization(const QDBusMessage &message, const QStringList &fileList)
{
    if (fileList.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }
    // TODO：暂时使用循环调用kss添加，kss有添加文件列表接口后修改为该接口
    for (auto filePath : fileList)
    {
        // 检测列表中是否存在相同文件
        QJsonParseError jsonError;
        auto jsonDoc = QJsonDocument::fromJson(KSSWrapper::getDefault()->getFiles().toUtf8(), &jsonError);
        if (jsonDoc.isNull())
        {
            KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        }

        auto jsonModules = jsonDoc.object().value(KSC_KSS_JK_DATA).toArray();
        for (const auto &module : jsonModules)
        {
            auto jsonMod = module.toObject();
            if (jsonMod.value(KSC_KSS_JK_DATA_PATH).toString() == filePath)
            {
                DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_TP_ADD_RECUR_FILE, message)
            }
        }
        // 添加文件
        QFileInfo fileInfo(filePath);
        auto fileName = fileInfo.fileName();
        KSSWrapper::getDefault()->addFile(fileName, filePath, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }

    emit ProtectedFilesChange();
    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::removeFPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }

    KSSWrapper::getDefault()->removeFile(filePath);
    emit ProtectedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::removeFPFilesAfterAuthorization(const QDBusMessage &message, const QStringList &fileList)
{
    if (fileList.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }
    // TODO：暂时使用循环调用kss移除，kss有添加文件列表接口后修改为该接口
    for (auto filePath : fileList)
    {
        KSSWrapper::getDefault()->removeFile(filePath);
    }

    emit ProtectedFilesChange();
    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::setStorageModeAfterAuthorization(const QDBusMessage &message, uint type, const QString &userPin)
{
    if (KSS_DEFAULT_USER_PIN != userPin)
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_USER_PIN_ERROR, message)
    }

    auto error = KSSWrapper::getDefault()->setStorageMode(KSCKSSTrustedStorageType(type), userPin);

    if (!error.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(KSCErrorCode::ERROR_CHANGE_STORAGE_MODE_FAILED, message)
    }

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::setTrustedStatusAfterAuthorization(const QDBusMessage &message, bool status)
{
    KSSWrapper::getDefault()->setTrustedStatus(status);

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

}  // namespace KS
