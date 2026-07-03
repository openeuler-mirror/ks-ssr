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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "src/ui/license/license-utils.h"

#include <kiran-log/qt5-log-i.h>
#include <exception>

namespace KS
{
LicenseUtils::LicenseUtils(QObject* parent) : QObject(parent)
{
    creatObjectName(LICENSE_OBJECT_NAME);
    QDBusConnection::systemBus().connect(LICENSE_HELPER_DBUS_NAME,
                                         LICENSE_OBJECT_OBJECT_PATH "/" LICENSE_OBJECT_NAME,
                                         LICENSE_OBJECT_DBUS_NAME,
                                         QLatin1String(SIGNAL_LICENSE_CHANGED),
                                         this,
                                         SLOT(licenseChange(bool)));
}

LicenseUtils::~LicenseUtils()
{
}

bool LicenseUtils::creatObjectName(const QString& objectName)
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(LICENSE_MANAGER_DBUS_NAME,
                                                                LICENSE_MANAGER_OBJECT_PATH,
                                                                LICENSE_MANAGER_DBUS_NAME,
                                                                METHOD_GET_LICENSE_OBJECT);
    msgMethodCall << objectName;

    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);
    QString errorMsg;
    if (msgReply.type() == QDBusMessage::ReplyMessage)
    {
        QList<QVariant> args = msgReply.arguments();
        if (args.size() < 1)
        {
            errorMsg = "arguments size < 1";
            goto failed;
        }

        return true;
    }
failed:
    KLOG_WARNING() << LICENSE_MANAGER_DBUS_NAME << METHOD_GET_LICENSE_OBJECT
                   << msgReply.errorName() << msgReply.errorMessage() << errorMsg;
    return false;
}

QString LicenseUtils::getLicense()
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(LICENSE_HELPER_DBUS_NAME,
                                                                LICENSE_OBJECT_OBJECT_PATH "/" LICENSE_OBJECT_NAME,
                                                                LICENSE_OBJECT_DBUS_NAME,
                                                                METHOD_GET_LICENSE);
    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);
    KLOG_DEBUG() << "getLicense msgReply " << msgReply;

    QString errMsg;
    if (msgReply.type() == QDBusMessage::ReplyMessage)
    {
        QList<QVariant> args = msgReply.arguments();
        if (args.size() < 1)
        {
            errMsg = "arguments size < 1";
            goto failed;
        }
        QVariant firstArg = args.takeFirst();
        return firstArg.toString();
    }

failed:
    KLOG_WARNING() << LICENSE_OBJECT_DBUS_NAME << METHOD_GET_LICENSE
                   << msgReply.errorName() << msgReply.errorMessage() << errMsg;

    return "";
}

bool LicenseUtils::activateByActivationCode(const QString& activation_Code, QString& errorMsg)
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(LICENSE_HELPER_DBUS_NAME,
                                                                LICENSE_OBJECT_OBJECT_PATH "/" LICENSE_OBJECT_NAME,
                                                                LICENSE_OBJECT_DBUS_NAME,
                                                                METHOD_ACTIVATE_BY_ACTIVATION_CODE);
    msgMethodCall << activation_Code;

    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);

    KLOG_DEBUG() << "activateByActivationCode msgReply " << msgReply;

    if (msgReply.type() == QDBusMessage::ErrorMessage)
    {
        goto failed;
    }
    return true;

failed:
    errorMsg = msgReply.errorName() + msgReply.errorMessage();
    KLOG_WARNING() << LICENSE_OBJECT_DBUS_NAME << METHOD_ACTIVATE_BY_ACTIVATION_CODE
                   << msgReply.errorName() << msgReply.errorMessage();
    return false;
}

void LicenseUtils::licenseChange(bool isChanged)
{
    emit licenseChanged(isChanged);
}

}  // namespace KS
