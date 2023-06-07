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
    vLayout->setSpacing(12);
    vLayout->setContentsMargins(12, 24, 12, 12);

    //存放usb接口状态
    m_usbLayout = new QGridLayout();
    m_usbLayout->setVerticalSpacing(12);
    m_usbLayout->setHorizontalSpacing(0);

    //存放键盘鼠标接口状态
    m_kbdMouseContent = new QWidget(this);
    m_kbdMouseLayout = new QGridLayout(m_kbdMouseContent);
    m_kbdMouseLayout->setVerticalSpacing(12);
    m_kbdMouseLayout->setHorizontalSpacing(0);
    m_kbdMouseLayout->setContentsMargins(0, 0, 0, 0);

    //存放其他接口状态
    m_gridLayout = new QGridLayout();
    m_gridLayout->setVerticalSpacing(12);
    m_gridLayout->setHorizontalSpacing(0);

    vLayout->addLayout(m_usbLayout);
    vLayout->addWidget(m_kbdMouseContent);
    vLayout->addLayout(m_gridLayout);

    vLayout->addStretch(1);
}

void SettingsDevice::updateUI()
{
    RETURN_IF_TRUE(m_interfaces.size() < 1);

    int count = 0;
    int kbdMouseCount = 1;
    bool usbEnabled = true;

    foreach (auto interface, m_interfaces)
    {
        if (interface.type <= InterfaceType::INTERFACE_TYPE_UNKNOWN ||
            interface.type >= InterfaceType::INTERFACE_TYPE_LAST)
        {
            continue;
        }

        auto interfaceName = DeviceUtils::interfaceTypeEnum2Str(interface.type);
        //在HDMI接口后面添加（重启后生效）
        if (interface.type == InterfaceType::INTERFACE_TYPE_HDMI)
        {
            interfaceName.append(tr(" (Effective after restart)"));
        }

        auto typeLabel = new QLabel(interfaceName, this);
        auto stateCheckBox = new QCheckBox(this);
        stateCheckBox->setChecked(interface.enable);
        stateCheckBox->setProperty(INTERFACE_TYPE_PROPERTY, interface.type);
        auto sparcerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);

        if (interface.type == InterfaceType::INTERFACE_TYPE_USB)
        {
            m_usbLayout->addWidget(typeLabel, 0, 0);
            m_usbLayout->addItem(sparcerItem, 0, 1);
            m_usbLayout->addWidget(stateCheckBox, 0, 2);
            usbEnabled = interface.enable;
        }
        else if (interface.type == InterfaceType::INTERFACE_TYPE_USB_KBD ||
                 interface.type == InterfaceType::INTERFACE_TYPE_USB_MOUSE)
        {
            m_kbdMouseLayout->addWidget(typeLabel, kbdMouseCount, 0);
            m_kbdMouseLayout->addItem(sparcerItem, kbdMouseCount, 1);
            m_kbdMouseLayout->addWidget(stateCheckBox, kbdMouseCount, 2);
            kbdMouseCount++;
        }
        else
        {
            m_gridLayout->addWidget(typeLabel, count, 0);
            m_gridLayout->addItem(sparcerItem, count, 1);
            m_gridLayout->addWidget(stateCheckBox, count, 2);
            count++;
        }

        connect(stateCheckBox, &QCheckBox::toggled, this, &SettingsDevice::handleInterfaceState);
    }

    m_kbdMouseContent->setVisible(!usbEnabled);
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

bool SettingsDevice::setInterfaceState(bool isEnable, InterfaceType type)
{
    auto reply = m_deviceManagerProxy->EnableInterface(type, isEnable);
    reply.waitForFinished();
    if (reply.isError())
    {
        POPUP_MESSAGE_DIALOG(reply.error().message());
        return false;
    }
    return true;
}

void SettingsDevice::handleInterfaceState(bool checked)
{
    auto state = checked;
    auto stateCheckBox = qobject_cast<QCheckBox *>(sender());
    auto type = (InterfaceType)stateCheckBox->property(INTERFACE_TYPE_PROPERTY).toInt();

    if (!setInterfaceState(state, type))
    {
        state = !checked;
        stateCheckBox->setChecked(state);
    }

    if (type == InterfaceType::INTERFACE_TYPE_USB)
    {
        m_kbdMouseContent->setVisible(!state);
    }
}
}  // namespace KS
