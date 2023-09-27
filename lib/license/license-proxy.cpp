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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "lib/license/license-proxy.h"

#include <kiran-log/qt5-log-i.h>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonParseError>
#include "include/ssr-marcos.h"

namespace KS
{
LicenseProxy::LicenseProxy(QObject* parent) : QObject(parent),
                                              m_isActivated(false),
                                              m_expiredTime(0)
{
    m_objectPath = getObjectPath(LICENSE_OBJECT_NAME);

    QDBusConnection::systemBus().connect(LICENSE_MANAGER_DBUS_NAME,
                                         m_objectPath,
                                         LICENSE_OBJECT_DBUS_NAME,
                                         QLatin1String(SIGNAL_LICENSE_CHANGED),
                                         this,
                                         SLOT(licenseChange(bool)));
}

QSharedPointer<LicenseProxy> LicenseProxy::getDefault()
{
    static QSharedPointer<LicenseProxy> licenseProxy = QSharedPointer<LicenseProxy>(new LicenseProxy);
    licenseProxy->updateLicense();
    return licenseProxy;
}

LicenseProxy::~LicenseProxy()
{
}

QString LicenseProxy::getObjectPath(const QString& objectName)
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(LICENSE_MANAGER_DBUS_NAME,
                                                                LICENSE_MANAGER_OBJECT_PATH,
                                                                LICENSE_MANAGER_DBUS_NAME,
                                                                METHOD_GET_LICENSE_OBJECT);
    msgMethodCall << objectName;
    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);
    KLOG_DEBUG() << "The reply of dbus method GetLicenseObject: " << msgReply;

    if (msgReply.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << "Failed to call dbus method GetLicenseObject: " << msgReply.errorMessage();
        return QString();
    }

    QList<QVariant> args = msgReply.arguments();
    if (args.size() < 1)
    {
        KLOG_WARNING() << "The size of arguments returned by GetLicenseObject is less than 1";
        return QString();
    }

    auto objectPath = args.takeFirst();
    QDBusObjectPath* path = reinterpret_cast<QDBusObjectPath*>(objectPath.data());
    return path->path();
}

void LicenseProxy::updateLicense()
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(LICENSE_MANAGER_DBUS_NAME,
                                                                m_objectPath,
                                                                LICENSE_OBJECT_DBUS_NAME,
                                                                METHOD_GET_LICENSE);
    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);
    KLOG_DEBUG() << "The reply of dbus method GetLicense: " << msgReply;

    if (msgReply.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << "Failed to call dbus method GetLicense: " << msgReply.errorMessage();
        return;
    }

    QList<QVariant> args = msgReply.arguments();
    if (args.size() < 1)
    {
        KLOG_WARNING() << "The size of arguments returned by GetLicense is less than 1";
        return;
    }

    QVariant firstArg = args.takeFirst();
    auto licenseInfoJson = firstArg.toString();
    //解析授权信息Json字符串
    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(licenseInfoJson.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser license information failed:" << jsonError.errorString();
        return;
    }

    auto data = jsonDoc.object();
    m_activationCode = data.value(LICENSE_JK_ACTIVATION_CODE).toString();
    m_machineCode = data.value(LICENSE_JK_MACHINE_CODE).toString();
    m_expiredTime = time_t(data.value(LICENSE_JK_EXPIRED_TIME).toVariant().toUInt());

    //获取激活状态
    auto activationStatus = (LicenseActivationStatus)data.value(LICENSE_JK_ACTIVATION_STATUS).toInt();
    m_isActivated = activationStatus == LAS_ACTIVATED;
}

bool LicenseProxy::activateByActivationCode(const QString& activation_Code, QString& errorMsg)
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(LICENSE_MANAGER_DBUS_NAME,
                                                                m_objectPath,
                                                                LICENSE_OBJECT_DBUS_NAME,
                                                                METHOD_ACTIVATE_BY_ACTIVATION_CODE);
    msgMethodCall << activation_Code;
    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);
    KLOG_DEBUG() << "The reply of dbus method ActivateByActivationCode: " << msgReply;

    if (msgReply.type() == QDBusMessage::ErrorMessage)
    {
        errorMsg = msgReply.errorMessage();
        KLOG_WARNING() << "Failed to call dbus method ActivateByActivationCode: " << errorMsg;
        return false;
    }
    return true;
}

bool LicenseProxy::isActivated()
{
    return m_isActivated;
}

QString LicenseProxy::getActivationCode()
{
    return m_activationCode;
}

QString LicenseProxy::getMachineCode()
{
    return m_machineCode;
}

time_t LicenseProxy::getExpiredTime()
{
    return m_expiredTime;
}

void LicenseProxy::licenseChange(bool)
{
    updateLicense();
    emit licenseChanged();
}

}  // namespace KS
