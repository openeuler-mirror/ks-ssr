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
LicenseDBus::LicenseDBus(QObject* parent) : QObject(parent), m_isActivated(false)
{
    creatObjectName(LICENSE_OBJECT_NAME);
    QDBusConnection::systemBus().connect(LICENSE_HELPER_DBUS_NAME,
                                         LICENSE_OBJECT_OBJECT_PATH "/" LICENSE_OBJECT_NAME,
                                         LICENSE_OBJECT_DBUS_NAME,
                                         QLatin1String(SIGNAL_LICENSE_CHANGED),
                                         this,
                                         SLOT(licenseChange(bool)));
}

QSharedPointer<LicenseDBus> LicenseDBus::getDefault()
{
    static QSharedPointer<LicenseDBus> licenseDbus = QSharedPointer<LicenseDBus>(new LicenseDBus);
    return licenseDbus;
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

QSharedPointer<LicenseInfo> LicenseDBus::getLicense()
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
    QSharedPointer<LicenseInfo> licenseInfo = QSharedPointer<LicenseInfo>(new LicenseInfo);
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
            auto licenseInfoJson = firstArg.toString();

            //解析授权信息Json字符串
            QJsonParseError jsonError;
            auto jsonDoc = QJsonDocument::fromJson(licenseInfoJson.toUtf8(), &jsonError);
            if (jsonDoc.isNull())
            {
                errorMsg = jsonError.errorString();
                break;
            }
            else
            {
                auto data = jsonDoc.object();
                licenseInfo.data()->activationCode = data.value(LICENSE_JK_ACTIVATION_CODE).toString();
                licenseInfo.data()->machineCode = data.value(LICENSE_JK_MACHINE_CODE).toString();
                licenseInfo.data()->activationStatus = (LicenseActivationStatus)data.value(LICENSE_JK_ACTIVATION_STATUS).toInt();
                licenseInfo.data()->expiredTime = time_t(data.value(LICENSE_JK_EXPIRED_TIME).toVariant().toUInt());

                //获取激活状态
                auto currTime = QDateTime::currentDateTime().toSecsSinceEpoch();
                m_isActivated = (licenseInfo.data()->activationStatus == LAS_ACTIVATED) && (currTime <= licenseInfo.data()->expiredTime);

                return licenseInfo;
            }
        }
    } while (0);

    KLOG_WARNING() << "get license information failed!"
                   << "dbus name: " << LICENSE_OBJECT_DBUS_NAME
                   << "method: " << METHOD_GET_LICENSE
                   << "error: " << msgReply.errorMessage() << errorMsg;

    return licenseInfo;
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

bool LicenseDBus::isActivated()
{
    return m_isActivated;
}

void LicenseDBus::licenseChange(bool)
{
    auto licenseInfo = getLicense();
    emit licenseChanged(licenseInfo);
}

}  // namespace KS
