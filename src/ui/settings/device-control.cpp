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

#include "device-control.h"
#include <QCheckBox>
#include <QLabel>
#include <QObject>
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/dm/utils.h"
#include "src/ui/settings/respond-dialog.h"

#define INTERFACE_TYPE_PROPERTY "interface type"

namespace KS
{
namespace Settings
{
DeviceControl::DeviceControl(QWidget *parent)
    : QWidget(parent),
      m_deviceManagerProxy(nullptr),
      m_gridLayout(nullptr),
      m_usbLayout(nullptr),
      m_kbdMouseLayout(nullptr),
      m_kbdMouseContent(nullptr),
      m_clickedCheckbox(nullptr),
      m_respondDlg(nullptr)
{
    initUI();

    m_deviceManagerProxy = new DeviceManagerProxy(SSR_DBUS_NAME,
                                                  SSR_DEVICE_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);
    m_interfaces = getInterfaces();
    insertInterfaceWidget();
}

DeviceControl::~DeviceControl()
{
}

void DeviceControl::initUI()
{
    auto vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(12);
    vLayout->setContentsMargins(12, 24, 12, 12);

    // 存放usb接口状态
    m_usbLayout = new QGridLayout();
    m_usbLayout->setVerticalSpacing(12);
    m_usbLayout->setHorizontalSpacing(0);

    // 存放键盘鼠标接口状态
    m_kbdMouseContent = new QWidget(this);
    m_kbdMouseLayout = new QGridLayout(m_kbdMouseContent);
    m_kbdMouseLayout->setVerticalSpacing(12);
    m_kbdMouseLayout->setHorizontalSpacing(0);
    m_kbdMouseLayout->setContentsMargins(0, 0, 0, 0);

    // 存放其他接口状态
    m_gridLayout = new QGridLayout();
    m_gridLayout->setVerticalSpacing(12);
    m_gridLayout->setHorizontalSpacing(0);

    vLayout->addLayout(m_usbLayout);
    vLayout->addWidget(m_kbdMouseContent);
    vLayout->addLayout(m_gridLayout);

    vLayout->addStretch(1);
}

void DeviceControl::insertInterfaceWidget()
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

        auto interfaceName = DM::Utils::interfaceTypeEnum2Str(interface.type);
        // 在HDMI接口后面添加（重启后生效）
        if (interface.type == InterfaceType::INTERFACE_TYPE_HDMI)
        {
            interfaceName.append(tr(" (Effective after restart)"));
        }

        auto typeLabel = new QLabel(interfaceName, this);
        auto stateCheckBox = new QCheckBox(this);
        stateCheckBox->setChecked(interface.enable);
        m_checkboxs.insert(interface.type, stateCheckBox);
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

        connect(stateCheckBox, &QCheckBox::clicked, this, &DeviceControl::setInterfaceState);
    }

    m_kbdMouseContent->setVisible(!usbEnabled);
}

void DeviceControl::update()
{
    m_interfaces.clear();
    m_interfaces = getInterfaces();

    foreach (auto interface, m_interfaces)
    {
        auto checkbox = m_checkboxs.value(interface.type);
        checkbox->setChecked(interface.enable);

        if (interface.type == InterfaceType::INTERFACE_TYPE_USB)
        {
            m_kbdMouseContent->setVisible(!interface.enable);
        }
    }
}

QList<Interface> DeviceControl::getInterfaces()
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

        auto interface = Interface{.type = (InterfaceType)data.value(SSR_DI_JK_TYPE).toInt(),
                                   .enable = data.value(SSR_DI_JK_ENABLE).toBool()};

        interfaces.push_back(interface);
    }
    return interfaces;
}

void DeviceControl::popupMessageDialog(const QString &text)
{
    if (!m_respondDlg)
    {
        m_respondDlg = new RespondDialog(this);
        connect(m_respondDlg, &RespondDialog::accepted, this, &DeviceControl::accept);
        connect(m_respondDlg, &RespondDialog::rejected, this, &DeviceControl::reject);
    }
    m_respondDlg->setMessage(text);
    int x = window()->x() + window()->width() / 4 + m_respondDlg->width() / 4;
    int y = window()->y() + window()->height() / 4 + m_respondDlg->height() / 8;
    m_respondDlg->move(x, y);
    m_respondDlg->show();
}

void DeviceControl::setInterfaceState(bool checked)
{
    auto state = checked;
    auto stateCheckBox = qobject_cast<QCheckBox *>(sender());
    auto type = m_checkboxs.key(stateCheckBox);
    m_clickedCheckbox = stateCheckBox;

    // 当关闭usb、鼠标、键盘设备时，弹出二次确认窗口
    if ((type == InterfaceType::INTERFACE_TYPE_USB ||
         type == InterfaceType::INTERFACE_TYPE_USB_KBD ||
         type == InterfaceType::INTERFACE_TYPE_USB_MOUSE) &&
        state == false)
    {
        popupMessageDialog(tr("Are you sure to close the %1 interface").arg(DM::Utils::interfaceTypeEnum2Str(type)));
    }
    else
    {
        accept();
    }
}

void DeviceControl::accept()
{
    auto type = m_checkboxs.key(m_clickedCheckbox);
    auto state = m_clickedCheckbox->isChecked();

    auto reply = m_deviceManagerProxy->EnableInterface(type, state);
    reply.waitForFinished();
    if (reply.isError())
    {
        POPUP_MESSAGE_DIALOG(reply.error().message());
    }

    // 更新界面checkbox状态
    update();
}

void DeviceControl::reject()
{
    m_clickedCheckbox->setChecked(!m_clickedCheckbox->isChecked());
}
}  // namespace Settings
}  // namespace KS
