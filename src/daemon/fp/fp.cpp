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

#include "src/daemon/fp/fp.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "config.h"
#include "include/ksc-error-i.h"
#include "include/ksc-i.h"
#include "include/ksc-marcos.h"
#include "lib/base/error.h"
#include "src/daemon/common/kss.h"
#include "src/daemon/common/polkit-proxy.h"
#include "src/daemon/fp_adaptor.h"

namespace KS
{
#define AUTH_FP_ADD_FILE "com.kylinsec.SC.FileProtected.AddFile"
#define AUTH_FP_REMOVE_FILE "com.kylinsec.SC.FileProtected.RemoveFile"

FP::FP(QObject *parent) : QObject(parent)
{
    m_dbusAdaptor = new FPAdaptor(this);

    init();
}
FP::~FP()
{
}

int FP::getInitialized() const
{
    return KSS::getDefault()->getInitialized();
}

CHECK_AUTH_WITH_1ARGS(FP, AddFile, onAddFile, AUTH_FP_ADD_FILE, const QString &)
CHECK_AUTH_WITH_1ARGS(FP, RemoveFile, onRemoveFile, AUTH_FP_REMOVE_FILE, const QString &)

QString FP::GetFiles()
{
    return KSS::getDefault()->getFiles();
}

QString FP::Search(const QString &pathKey)
{
    if (pathKey.isEmpty())
    {
        sendErrorReply(QDBusError::InvalidArgs, KSC_ERROR2STR(KSCErrorCode::ERROR_COMMON_INVALID_ARGS));
        return QString();
    }

    QJsonDocument resultJsonDoc;
    QJsonArray jsonArr;

    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(KSS::getDefault()->getFiles().toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser kernel protected information failed: " << jsonError.errorString();
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

void FP::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(KSC_FP_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}

void FP::onAddFile(const QDBusMessage &message, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        auto replyMessage = message.createErrorReply(QDBusError::InvalidArgs, KSC_ERROR2STR(KSCErrorCode::ERROR_COMMON_INVALID_ARGS));
        QDBusConnection::systemBus().send(replyMessage);
        return;
    }

    QFileInfo fileInfo(filePath);
    auto fileName = fileInfo.fileName();
    KSS::getDefault()->addFile(fileName, filePath, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

void FP::onRemoveFile(const QDBusMessage &message, const QString &filePath)
{
    if (filePath.isEmpty())
    {
        auto replyMessage = message.createErrorReply(QDBusError::InvalidArgs, KSC_ERROR2STR(KSCErrorCode::ERROR_COMMON_INVALID_ARGS));
        QDBusConnection::systemBus().send(replyMessage);
        return;
    }
    KSS::getDefault()->removeFile(filePath);

    auto replyMessage = message.createReply();
    QDBusConnection::systemBus().send(replyMessage);
}

}  // namespace KS
