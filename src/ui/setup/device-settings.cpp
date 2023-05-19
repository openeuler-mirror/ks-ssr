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

#include "device-settings.h"
#include <QCheckBox>
#include <QLabel>
#include <QObject>
#include "include/ksc-marcos.h"
#include "src/ui/common/message-dialog.h"
#include "src/ui/device/device-utils.h"

#define INTERFACE_TYPE_PROPERTY "interface type"

namespace KS
{
DeviceSettings::DeviceSettings(QWidget *parent) : QWidget(parent),
                                                  m_deviceManagerProxy(nullptr)
{
    initUI();

    m_deviceManagerProxy = new DeviceManagerProxy(KSC_DBUS_NAME,
                                                  KSC_DEVICE_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);
    m_interfaces = getInterfaces();
    updateUI();
}

DeviceSettings::~DeviceSettings()
{
}

void DeviceSettings::initUI()
{
    auto vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(12, 24, 12, 12);

    m_gridLayout = new QGridLayout();
    m_gridLayout->setVerticalSpacing(12);
    m_gridLayout->setHorizontalSpacing(0);

    vLayout->addLayout(m_gridLayout);
    vLayout->addStretch(1);
}

void DeviceSettings::updateUI()
{
    RETURN_IF_TRUE(m_interfaces.size() < 1);

    int count = 0;
    foreach (auto interface, m_interfaces)
    {
        auto typeLabel = new QLabel(DeviceUtils::interfaceTypeEnum2Str(interface.type), this);
        auto stateCheckBox = new QCheckBox(this);
        stateCheckBox->setChecked(interface.enable);
        stateCheckBox->setProperty(INTERFACE_TYPE_PROPERTY, interface.type);
        auto sparcerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_gridLayout->addWidget(typeLabel, count, 0);
        m_gridLayout->addItem(sparcerItem, count, 1);
        m_gridLayout->addWidget(stateCheckBox, count, 2);

        connect(stateCheckBox, &QCheckBox::toggled, this, &DeviceSettings::handleInterfaceState);
        count++;
    }
}

QList<Interface> DeviceSettings::getInterfaces()
{
    QList<Interface> interfaces;
    auto reply = m_deviceManagerProxy->GetInterfaces();
    reply.waitForFinished();
    auto interfacesJson = reply.value();
    KLOG_DEBUG() << "The reply of dbus method GetInterfaces:" << interfacesJson;

    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(interfacesJson.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files information failed: " << jsonError.errorString();
        return interfaces;
    }

    auto jsonDataArray = jsonDoc.array();
    for (auto jsonData : jsonDataArray)
    {
        auto data = jsonData.toObject();

        auto interface = Interface{.type = (InterfaceType)data.value(KSC_DI_JK_TYPE).toInt(),
                                   .enable = data.value(KSC_DI_JK_ENABLE).toBool()};

        interfaces.push_back(interface);
    }
    return interfaces;
}

void DeviceSettings::setInterfaceState(bool isEnable, InterfaceType type)
{
    RETURN_IF_TRUE(type == InterfaceType::INTERFACE_TYPE_UNKNOWN);

    QString errMsg;

    m_deviceManagerProxy->EnableInterface(type, isEnable);

    // TODO: 这里改成监控dbus错误消息，这种弹框用到的地方比较多，可以做一下封装，没必要每个地方都写一遍
    if (!errMsg.isEmpty())
    {
        auto msgDialog = new MessageDialog(this);
        msgDialog->setMessage(errMsg);
        int x = window()->x() + window()->width() / 4 + msgDialog->width() / 4;
        int y = window()->y() + window()->height() / 4 + msgDialog->height() / 4;
        msgDialog->move(x, y);
        msgDialog->show();
        return;
    }
}

void DeviceSettings::handleInterfaceState(bool checked)
{
    auto stateCheckBox = qobject_cast<QCheckBox *>(sender());
    auto type = (InterfaceType)stateCheckBox->property(INTERFACE_TYPE_PROPERTY).toInt();

    setInterfaceState(checked, type);
}
}  // namespace KS
