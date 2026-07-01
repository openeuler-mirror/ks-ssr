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
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#include "src/daemon/dm/usb-device.h"
#include <qt5-log-i.h>
#include <QFile>
#include "ssr-i.h"
#include "ssr-marcos.h"
#include "usb-device-description.h"

namespace KS
{
namespace DM
{
#define USB_INTERFACE_CLASS_RESERVED 0x00
#define USB_INTERFACE_CLASS_AUDIO 0x01
#define USB_INTERFACE_CLASS_COMMUNICATIONS 0x02
#define USB_INTERFACE_CLASS_HUMAN_INTERFACE 0x03
#define USB_INTERFACE_CLASS_MONITOR 0x04
#define USB_INTERFACE_CLASS_PHYSICAL_INTERFACE 0x05
#define USB_INTERFACE_CLASS_POWER 0x06
#define USB_INTERFACE_CLASS_IMAGE 0x06
#define USB_INTERFACE_CLASS_PRINTER 0x07
#define USB_INTERFACE_CLASS_STORAGE 0x08
#define USB_INTERFACE_CLASS_HUB 0x09
#define USB_INTERFACE_CLASS_CDC_DATA 0x0A
#define USB_INTERFACE_CLASS_SMART_CARD 0x0B
#define USB_INTERFACE_CLASS_CONTENT_SECURITY 0x0D
#define USB_INTERFACE_CLASS_VIDEO 0x0E
#define USB_INTERFACE_CLASS_PERSONAL_HEALTHCARE 0x0F
#define USB_INTERFACE_CLASS_AUDIO_VIDEO 0x10
#define USB_INTERFACE_CLASS_BILLBOARD 0x11
#define USB_INTERFACE_CLASS_DIAGNOSTIC_INTERFACE 0xDC
#define USB_INTERFACE_CLASS_WIRELESS_CONTROLLER 0xE0
#define USB_INTERFACE_CLASS_MISCELLANEOUS 0xEF
#define USB_INTERFACE_CLASS_APPLICATION_SPECIFIC 0xFE
#define USB_INTERFACE_CLASS_VENDOR_SPECIFIC 0xFF

#define USB_HID_INTERFACE_SUB_CLASS_BOOT 0x01
#define USB_HID_INTERFACE_PROTOCOL_KEYBOARD 0x01
#define USB_HID_INTERFACE_PROTOCOL_MOUSE 0x02

#define USB_WC_INTERFACE_SUB_CLASS_BOOT 0x01
#define USB_WC_INTERFACE_PROTOCOL_BLUETOOTH 0x01

#define USB_DEVICE_CLASS_HUB 0x09

QMap<QString, int> USBDevice::m_fixedTypes{{"096e:0202", DEVICE_TYPE_STORAGE}};

USBDevice::USBDevice(const QString &syspath, QObject *parent)
    : Device(syspath, parent)
{
    m_devConfig = Configuration::instance();

    this->init();
    this->initPermission();
}

USBDevice::~USBDevice() {}

void USBDevice::init()
{
    auto device = this->getSDDevcie();
    auto desc = USBDeviceDescription::instance();

    m_idVendor = device->getSysattrValue("idVendor");
    m_idProduct = device->getSysattrValue("idProduct");

    m_product = device->getSysattrValue("product");
    if (m_product.isNull())
    {
        m_product = desc->getProductDesc(m_idVendor, m_idProduct);
    }

    m_manufacturer = device->getSysattrValue("manufacturer");
    if (m_manufacturer.isNull())
    {
        m_manufacturer = desc->getManufacturerDesc(m_idVendor);
    }

    m_uid = "USB" + m_idVendor + m_idProduct;

    auto id = QString("%1:%2:%3-%4").arg(m_idVendor, m_idProduct, device->getSysattrValue("busnum"), device->getSysattrValue("devnum"));
    this->setID(id);
    this->setName(m_product);
    this->setType(this->parseDeviceType());
    this->setInterfaceType(INTERFACE_TYPE_USB);
}

int USBDevice::parseDeviceType()
{
    auto type = m_fixedTypes.value(m_idVendor + ":" + m_idProduct,
                                   this->parseDeviceInterfaceClassType());

    RETURN_VAL_IF_TRUE(type != DEVICE_TYPE_OTHER, type)

    return this->deviceClass2DeviceType();
}

int USBDevice::deviceClass2DeviceType()
{
    auto bDeviceClass = this->getSDDevcie()->getSysattrValue("bDeviceClass").toInt(nullptr, 16);

    RETURN_VAL_IF_TRUE(bDeviceClass == USB_DEVICE_CLASS_HUB, DEVICE_TYPE_HUB)

    return DEVICE_TYPE_OTHER;
}

int USBDevice::parseDeviceInterfaceClassType()
{
    InterfaceClass interfaceClass;
    auto syspath = this->getSyspath();
    auto sysname = this->getSDDevcie()->getSysname();

    RETURN_VAL_IF_TRUE(syspath.isNull() || sysname.isNull(), DEVICE_TYPE_OTHER)

    auto childDeviceSyspath = QString("%1/%2:1.0").arg(syspath, sysname);

    RETURN_VAL_IF_FALSE(QFile::exists(childDeviceSyspath), DEVICE_TYPE_OTHER)

    SDDevice childDevice(childDeviceSyspath);

    interfaceClass.bInterfaceClass = childDevice.getSysattrValue("bInterfaceClass").toInt(nullptr, 16);
    interfaceClass.bInterfaceSubClass = childDevice.getSysattrValue("bInterfaceSubClass").toInt(nullptr, 16);
    interfaceClass.bInterfaceProtocol = childDevice.getSysattrValue("bInterfaceProtocol").toInt(nullptr, 16);

    return this->interfaceProtocol2DevcieType(interfaceClass);
}

int USBDevice::interfaceProtocol2DevcieType(const InterfaceClass &interface)
{
    int type = DEVICE_TYPE_OTHER;

    switch (interface.bInterfaceClass)
    {
    case USB_INTERFACE_CLASS_AUDIO:
        type = DEVICE_TYPE_AUDIO;
        break;

    case USB_INTERFACE_CLASS_COMMUNICATIONS:
        type = DEVICE_TYPE_COMMUNICATIONS;
        break;

    case USB_INTERFACE_CLASS_HUMAN_INTERFACE:
        type = this->hidProtocol2DevcieType(interface);
        break;

    case USB_INTERFACE_CLASS_PRINTER:
        type = DEVICE_TYPE_PRINTER;
        break;

    case USB_INTERFACE_CLASS_STORAGE:
        type = DEVICE_TYPE_STORAGE;
        break;

    case USB_INTERFACE_CLASS_HUB:
        type = DEVICE_TYPE_HUB;
        break;

    case USB_INTERFACE_CLASS_VIDEO:
        type = DEVICE_TYPE_VIDEO;
        break;

    case USB_INTERFACE_CLASS_WIRELESS_CONTROLLER:
        type = this->wcProtocol2DevcieType(interface);
        break;

    default:
        break;
    }

    return type;
}

int USBDevice::hidProtocol2DevcieType(const InterfaceClass &interface)
{
    // 默认输入设备为鼠标
    auto type = DEVICE_TYPE_MOUSE;

    if (interface.bInterfaceSubClass == USB_HID_INTERFACE_SUB_CLASS_BOOT &&
        interface.bInterfaceProtocol == USB_HID_INTERFACE_PROTOCOL_KEYBOARD)
    {
        type = DEVICE_TYPE_KEYBOARD;
    }

    return type;
}

int USBDevice::wcProtocol2DevcieType(const InterfaceClass &interface)
{
    auto type = DEVICE_TYPE_BLUETOOTH;
    if (interface.bInterfaceSubClass == USB_WC_INTERFACE_SUB_CLASS_BOOT &&
        interface.bInterfaceProtocol == USB_WC_INTERFACE_PROTOCOL_BLUETOOTH)
    {
        type = DEVICE_TYPE_BLUETOOTH;
    }

    return type;
}

#pragma message("将 USB 的默认权限设置为启用")
void USBDevice::initPermission()
{
    auto setting = m_devConfig->getDeviceSetting(m_uid);

    if (setting == nullptr)
    {
        this->setState(DEVICE_STATE_UNAUTHORIED);
        this->setDeviceAuthorized();
        return;
    }

    this->setPermission(QSharedPointer<Permission>(new Permission{
        .read = setting->read,
        .write = setting->write,
        .execute = setting->execute,
    }));

    this->setState((setting->enable) ? DEVICE_STATE_ENABLE : DEVICE_STATE_DISABLE);
    this->setDeviceAuthorized();
}

bool USBDevice::setEnable(bool enable)
{
    DeviceSetting setting;

    setting.uid = m_uid;
    setting.id = this->getID();
    setting.name = this->getName();
    setting.idVendor = m_idVendor;
    setting.idProduct = m_idProduct;
    setting.enable = enable;
    setting.type = this->getType();
    setting.interfaceType = this->getInterfaceType();
    setting.read = this->getPermission()->read;
    setting.write = this->getPermission()->write;
    setting.execute = this->getPermission()->execute;

    m_devConfig->addSetting(setting);
    this->setState((setting.enable) ? DEVICE_STATE_ENABLE : DEVICE_STATE_DISABLE);

    this->setDeviceAuthorized();

    return true;
}

void USBDevice::update()
{
    auto type = this->parseDeviceType();

    if (type != DEVICE_TYPE_OTHER)
    {
        this->setType(type);
    }

    this->setDeviceAuthorized();
}

void USBDevice::setDeviceAuthorized()
{
    auto filePath = QString("%1/authorized").arg(this->getSyspath());
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "Cannot open file " << filePath;
        return;
    }

    QTextStream out(&file);
    out << (this->isEnable() ? "1" : "0");
    file.close();
}

bool USBDevice::isEnable()
{
    auto usbEnable = m_devConfig->isIFCEnable(INTERFACE_TYPE_USB);
    auto kdbEnable = m_devConfig->isIFCEnable(INTERFACE_TYPE_USB_KBD);
    auto mouseEnable = m_devConfig->isIFCEnable(INTERFACE_TYPE_USB_MOUSE);
    auto state = this->getState();
    auto type = this->getType();

    // 禁用设备
    RETURN_VAL_IF_TRUE(state == DEVICE_STATE_DISABLE, false)

    // 全局USB接口关闭
    if (!usbEnable)
    {
        // 鼠标键盘启用时，启用Hub
        RETURN_VAL_IF_TRUE((kdbEnable || mouseEnable) && type == DEVICE_TYPE_HUB, true)
        // 启用键盘
        RETURN_VAL_IF_TRUE(kdbEnable && type == DEVICE_TYPE_KEYBOARD, true)
        // 启用鼠标
        RETURN_VAL_IF_TRUE(mouseEnable && type == DEVICE_TYPE_MOUSE, true)

        return false;
    }

    // 默认鼠标，键盘，Hub可以使用
    RETURN_VAL_IF_TRUE((type == DEVICE_TYPE_HUB ||
                        type == DEVICE_TYPE_KEYBOARD ||
                        type == DEVICE_TYPE_MOUSE),
                       true)

    return state == DEVICE_STATE_ENABLE;
}
}  // namespace DM
}  // namespace KS
