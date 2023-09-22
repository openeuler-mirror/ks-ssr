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

#include "settings-device.h"
#include <QCheckBox>
#include <QLabel>
#include <QObject>
#include "src/ui/common/ksc-marcos-ui.h"
#include "src/ui/device/device-utils.h"

#define INTERFACE_TYPE_PROPERTY "interface type"

namespace KS
{
SettingsDevice::SettingsDevice(QWidget *parent) : QWidget(parent),
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

SettingsDevice::~SettingsDevice()
{
}

void SettingsDevice::initUI()
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

void SettingsDevice::updateUI()
{
    RETURN_IF_TRUE(m_interfaces.size() < 1);

    int count = 0;
    foreach (auto interface, m_interfaces)
    {
        auto interfaceName = DeviceUtils::interfaceTypeEnum2Str(interface.type);
        //在HDMI接口后面添加（重启后生效）
        if (interfaceName == DeviceUtils::interfaceTypeEnum2Str(InterfaceType::INTERFACE_TYPE_HDMI))
        {
            interfaceName.append(tr(" (Effective after restart)"));
        }

        auto typeLabel = new QLabel(interfaceName, this);
        auto stateCheckBox = new QCheckBox(this);
        stateCheckBox->setChecked(interface.enable);
        stateCheckBox->setProperty(INTERFACE_TYPE_PROPERTY, interface.type);
        auto sparcerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_gridLayout->addWidget(typeLabel, count, 0);
        m_gridLayout->addItem(sparcerItem, count, 1);
        m_gridLayout->addWidget(stateCheckBox, count, 2);

        connect(stateCheckBox, &QCheckBox::toggled, this, &SettingsDevice::handleInterfaceState);
        count++;
    }
}

QList<Interface> SettingsDevice::getInterfaces()
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

void SettingsDevice::setInterfaceState(bool isEnable, InterfaceType type)
{
    RETURN_IF_TRUE(type == InterfaceType::INTERFACE_TYPE_UNKNOWN);

    QString errMsg;

    m_deviceManagerProxy->EnableInterface(type, isEnable);

    // TODO: 这里改成监控dbus错误消息，这种弹框用到的地方比较多，可以做一下封装，没必要每个地方都写一遍
    if (!errMsg.isEmpty())
    {
        POPUP_MESSAGE_DIALOG(errMsg);
        return;
    }
}

void SettingsDevice::handleInterfaceState(bool checked)
{
    auto stateCheckBox = qobject_cast<QCheckBox *>(sender());
    auto type = (InterfaceType)stateCheckBox->property(INTERFACE_TYPE_PROPERTY).toInt();

    setInterfaceState(checked, type);
}
}  // namespace KS