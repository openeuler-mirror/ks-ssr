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

#include "src/ui/license/license-dbus.h"

#include <kiran-log/qt5-log-i.h>
#include <exception>
#include "include/ksc-marcos.h"

namespace KS
{
LicenseDBus::LicenseDBus(QObject* parent) : QObject(parent)
{
    creatObjectName(LICENSE_OBJECT_NAME);
    QDBusConnection::systemBus().connect(LICENSE_HELPER_DBUS_NAME,
                                         LICENSE_OBJECT_OBJECT_PATH "/" LICENSE_OBJECT_NAME,
                                         LICENSE_OBJECT_DBUS_NAME,
                                         QLatin1String(SIGNAL_LICENSE_CHANGED),
                                         this,
                                         SLOT(licenseChange(bool)));
}

LicenseDBus::~LicenseDBus()
{
}

bool LicenseDBus::creatObjectName(const QString& objectName)
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
    do
    {
        if (msgReply.type() == QDBusMessage::ReplyMessage)
        {
            QList<QVariant> args = msgReply.arguments();
            RETURN_VAL_IF_TRUE(args.size() >= 1, true);

            errorMsg = tr("arguments size < 1");
            break;
        }

    } while (0);

    KLOG_WARNING() << "create object name failed!"
                   << "dbus name: " << LICENSE_MANAGER_DBUS_NAME
                   << "method: " << METHOD_GET_LICENSE_OBJECT
                   << "error: " << msgReply.errorMessage() << errorMsg;
    return false;
}

QString LicenseDBus::getLicense()
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(LICENSE_HELPER_DBUS_NAME,
                                                                LICENSE_OBJECT_OBJECT_PATH "/" LICENSE_OBJECT_NAME,
                                                                LICENSE_OBJECT_DBUS_NAME,
                                                                METHOD_GET_LICENSE);
    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);
    KLOG_DEBUG() << "getLicense msgReply: " << msgReply;

    QString errorMsg;
    do
    {
        if (msgReply.type() == QDBusMessage::ReplyMessage)
        {
            QList<QVariant> args = msgReply.arguments();
            if (args.size() < 1)
            {
                errorMsg = tr("arguments size < 1");
                break;
            }
            QVariant firstArg = args.takeFirst();
            return firstArg.toString();
        }
    } while (0);

    KLOG_WARNING() << "get license information failed!"
                   << "dbus name: " << LICENSE_OBJECT_DBUS_NAME
                   << "method: " << METHOD_GET_LICENSE
                   << "error: " << msgReply.errorMessage() << errorMsg;

    return "";
}

bool LicenseDBus::activateByActivationCode(const QString& activation_Code, QString& errorMsg)
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(LICENSE_HELPER_DBUS_NAME,
                                                                LICENSE_OBJECT_OBJECT_PATH "/" LICENSE_OBJECT_NAME,
                                                                LICENSE_OBJECT_DBUS_NAME,
                                                                METHOD_ACTIVATE_BY_ACTIVATION_CODE);
    msgMethodCall << activation_Code;

    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);

    KLOG_DEBUG() << "activateByActivationCode msgReply: " << msgReply;

    RETURN_VAL_IF_TRUE(msgReply.type() != QDBusMessage::ErrorMessage, true);

    errorMsg = tr("activate by activation code failed:\n dbus name: %1\n error: %2:")
                   .arg(LICENSE_OBJECT_DBUS_NAME)
                   .arg(msgReply.errorMessage());
    KLOG_WARNING() << errorMsg;
    return false;
}

void LicenseDBus::licenseChange(bool)
{
    emit licenseChanged();
}

}  // namespace KS
