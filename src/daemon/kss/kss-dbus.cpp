/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
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
#include "include/ssr-error-i.h"
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"
#include "lib/base/error.h"
#include "src/daemon/common/polkit-proxy.h"
#include "src/daemon/kss/kss-wrapper.h"
#include "src/daemon/kss_dbus_adaptor.h"

namespace KS
{
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

CHECK_AUTH_WITH_1ARGS(KSSDbus, AddTrustedFile, addTPFileAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, AddTrustedFiles, addTPFilesAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, const QStringList &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, RemoveTrustedFile, removeTPFileAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, RemoveTrustedFiles, removeTPFilesAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, const QStringList &)
CHECK_AUTH_WITH_2ARGS(KSSDbus, ProhibitUnloading, prohibitUnloadingAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, bool, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, AddProtectedFile, addFPFileAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, AddProtectedFiles, addFPFilesAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, const QStringList &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, RemoveProtectedFile, removeFPFileAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, RemoveProtectedFiles, removeFPFilesAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, const QStringList &)
CHECK_AUTH_WITH_2ARGS(KSSDbus, SetStorageMode, setStorageModeAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, uint, const QString &)
CHECK_AUTH_WITH_1ARGS(KSSDbus, SetTrustedStatus, setTrustedStatusAfterAuthorization, SSR_PERMISSION_AUTHENTICATION, bool);

QString KSSDbus::GetTrustedFiles(uint type)
{
    return KSSWrapper::getDefault()->getTrustedFiles(SSRKSSTrustedFileType(type));
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
                                        SSRErrorCode::ERROR_COMMON_INVALID_ARGS,
                                        message())
    }

    RETURN_VAL_IF_TRUE(KSSType(searchType) == KSSType::KSS_TYPE_NONE, QString())
    QString fileList;
    QJsonDocument resultJsonDoc;
    QJsonArray jsonArr;
    QJsonParseError jsonError;

    if (KSSType(searchType) == KSSType::KSS_TYPE_TP_EXECUTE)
    {
        fileList = KSSWrapper::getDefault()->getTrustedFiles(SSRKSSTrustedFileType::SSR_KSS_TRUSTED_FILE_TYPE_EXECUTE);
    }
    else if (KSSType(searchType) == KSSType::KSS_TYPE_TP_KERNEL)
    {
        fileList = KSSWrapper::getDefault()->getTrustedFiles(SSRKSSTrustedFileType::SSR_KSS_TRUSTED_FILE_TYPE_KERNEL);
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

    auto jsonModules = jsonDoc.object().value(SSR_KSS_JK_DATA).toArray();
    for (const auto &module : jsonModules)
    {
        auto jsonMod = module.toObject();

        // 通过输入的pathKey，判断list中path字段是否包含pathKey
        if (jsonMod.value(SSR_KSS_JK_DATA_PATH).toString().contains(pathKey))
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

    if (!connection.registerObject(SSR_KSS_INIT_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}

QJsonDocument KSSDbus::fileProtectedListToJsonDocument(const QStringList &fileList)
{
    QJsonDocument document;
    QJsonArray jsonArray;
    for (auto filePath : fileList)
    {
        QFileInfo fileInfo(filePath);
        auto fileName = fileInfo.fileName();
        QJsonObject jsonObj{
            {SSR_KSS_JK_DATA_FILE_NAME, fileName},
            {SSR_KSS_JK_DATA_PATH, filePath},
            {SSR_KSS_JK_DATA_ADD_TIME, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")}};

        jsonArray.push_back(jsonObj);
    }
    QJsonObject jsonObj{
        {SSR_KSS_JK_RES, 0},
        {SSR_KSS_JK_COUNT, fileList.size()},
        {SSR_KSS_JK_DATA, jsonArray}};
    document.setObject(jsonObj);

    return document;
}

bool KSSDbus::checkFPDuplicateFiles(const QString &filePath, const QDBusMessage &message)
{
    // 检测列表中是否存在相同文件
    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(KSSWrapper::getDefault()->getFiles().toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_FAILED, message)
    }

    auto jsonModules = jsonDoc.object().value(SSR_KSS_JK_DATA).toArray();
    for (const auto &module : jsonModules)
    {
        auto jsonMod = module.toObject();
        if (jsonMod.value(SSR_KSS_JK_DATA_PATH).toString() == filePath)
        {
            DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_TP_ADD_RECUR_FILE, message)
        }
    }
    return true;
}

QJsonDocument KSSDbus::trustedProtectedListToJsonDocument(const QStringList &fileList)
{
    QJsonDocument document;
    QJsonArray jsonArray;

    for (auto filePath : fileList)
    {
        QJsonObject jsonObj{
            {SSR_KSS_JK_DATA_PATH, filePath},
            {SSR_KSS_JK_DATA_TYPE, 0},
            {SSR_KSS_JK_DATA_STATUS, 0},
            {SSR_KSS_JK_DATA_HASH, ""},
            {SSR_KSS_JK_DATA_GUARD, 0}};

        jsonArray.push_back(jsonObj);
    }
    QJsonObject jsonObj{
        {SSR_KSS_JK_RES, 0},
        {SSR_KSS_JK_COUNT, fileList.size()},
        {SSR_KSS_JK_DATA, jsonArray}};
    document.setObject(jsonObj);

    return document;
}

void KSSDbus::addTPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }

    auto output = KSSWrapper::getDefault()->addTrustedFile(filePath);
    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(output.toUtf8(), &jsonError);

    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_FAILED, message)
    }

    if (jsonDoc.object().value(SSR_KSS_JK_COUNT).toInt() == 0)
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TP_ADD_INVALID_FILE, message)
    }
    emit TrustedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::addTPFilesAfterAuthorization(const QDBusMessage &message, const QStringList &fileList)
{
    if (fileList.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }
    QJsonDocument jsonDataDoc = trustedProtectedListToJsonDocument(fileList);
    auto output = KSSWrapper::getDefault()->addTrustedFiles(QString(jsonDataDoc.toJson()));

    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(output.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_FAILED, message)
    }

    if (jsonDoc.object().value(SSR_KSS_JK_COUNT).toInt() == 0)
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TP_ADD_INVALID_FILE, message)
    }

    emit TrustedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::removeTPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message)
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
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }
    QJsonDocument jsonDoc = trustedProtectedListToJsonDocument(fileList);
    KSSWrapper::getDefault()->removeTrustedFiles(QString(jsonDoc.toJson()));

    emit TrustedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::prohibitUnloadingAfterAuthorization(const QDBusMessage &message, bool prohibited, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }

    KSSWrapper::getDefault()->prohibitUnloading(prohibited, filePath);
    // emit TrustedFilesChange();

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::addFPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }
    if (!checkFPDuplicateFiles(filePath, message))
    {
        return;
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
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }

    QJsonDocument jsonDoc = fileProtectedListToJsonDocument(fileList);
    KSSWrapper::getDefault()->addFiles(QString(jsonDoc.toJson()));

    emit ProtectedFilesChange();
    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::removeFPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message)
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
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message)
    }
    QJsonDocument jsonDoc = fileProtectedListToJsonDocument(fileList);
    KSSWrapper::getDefault()->removeFiles(QString(jsonDoc.toJson()));

    emit ProtectedFilesChange();
    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void KSSDbus::setStorageModeAfterAuthorization(const QDBusMessage &message, uint type, const QString &userPin)
{
    if (KSS_DEFAULT_USER_PIN != userPin)
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_USER_PIN_ERROR, message)
    }

    auto error = KSSWrapper::getDefault()->setStorageMode(SSRKSSTrustedStorageType(type), userPin);

    if (!error.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_CHANGE_STORAGE_MODE_FAILED, message)
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
