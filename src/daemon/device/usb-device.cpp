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
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#include "src/daemon/device/usb-device.h"
#include <qt5-log-i.h>
#include <QFile>
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "usb-device-description.h"

namespace KS
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

#define USB_DEVICE_CLASS_HUB 0x09

USBDevice::USBDevice(const QString &syspath, QObject *parent)
    : Device(syspath, parent)
{
    m_ruleManager = DeviceRuleManager::instance();

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

    auto id = m_idVendor + ":" + m_idProduct;
    this->setID(id);
    this->setName(m_product);
    this->setType(this->parseDeviceType());
    this->setInterfaceType(INTERFACE_TYPE_USB);
}

int USBDevice::parseDeviceType()
{
    auto type = this->parseDeviceInterfaceClassType();

    RETURN_VAL_IF_TRUE(type != DEVICE_TYPE_UNKNOWN, type)

    return this->deviceClass2DeviceType();
}

int USBDevice::deviceClass2DeviceType()
{
    auto bDeviceClass = this->getSDDevcie()->getSysattrValue("bDeviceClass").toInt(nullptr, 16);

    RETURN_VAL_IF_TRUE(bDeviceClass == USB_DEVICE_CLASS_HUB, DEVICE_TYPE_HUB)

    return DEVICE_TYPE_UNKNOWN;
}

int USBDevice::parseDeviceInterfaceClassType()
{
    InterfaceClass interfaceClass;
    auto syspath = this->getSyspath();
    auto sysname = this->getSDDevcie()->getSysname();

    RETURN_VAL_IF_TRUE(syspath.isNull() || sysname.isNull(), DEVICE_TYPE_UNKNOWN)

    auto childDeviceSyspath = QString::asprintf("%s/%s:1.0",
                                                syspath.toStdString().c_str(),
                                                sysname.toStdString().c_str());

    RETURN_VAL_IF_FALSE(QFile::exists(childDeviceSyspath), DEVICE_TYPE_UNKNOWN)

    SDDevice childDevice(childDeviceSyspath);

    interfaceClass.bInterfaceClass = childDevice.getSysattrValue("bInterfaceClass").toInt(nullptr, 16);
    interfaceClass.bInterfaceSubClass = childDevice.getSysattrValue("bInterfaceSubClass").toInt(nullptr, 16);
    interfaceClass.bInterfaceProtocol = childDevice.getSysattrValue("bInterfaceProtocol").toInt(nullptr, 16);

    return this->interfaceProtocol2DevcieType(interfaceClass);
}

int USBDevice::interfaceProtocol2DevcieType(const InterfaceClass &interface)
{
    int type = DEVICE_TYPE_UNKNOWN;

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
        type = DEVICE_TYPE_DISK;
        break;

    case USB_INTERFACE_CLASS_HUB:
        type = DEVICE_TYPE_HUB;
        break;

    case USB_INTERFACE_CLASS_VIDEO:
        type = DEVICE_TYPE_VIDEO;
        break;

    default:
        break;
    }

    return type;
}

int USBDevice::hidProtocol2DevcieType(const InterfaceClass &interface)
{
    auto type = DEVICE_TYPE_UNKNOWN;

    if (interface.bInterfaceSubClass == USB_HID_INTERFACE_SUB_CLASS_BOOT)
    {
        if (interface.bInterfaceProtocol == USB_HID_INTERFACE_PROTOCOL_KEYBOARD)
        {
            type = DEVICE_TYPE_KEYBOARD;
        }
        else if (interface.bInterfaceProtocol == USB_HID_INTERFACE_PROTOCOL_MOUSE)
        {
            type = DEVICE_TYPE_MOUSE;
        }
    }

    return type;
}

void USBDevice::initPermission()
{
    auto rule = m_ruleManager->getRule(m_uid);

    if (rule == nullptr)
    {
        this->setState(DEVICE_STATE_UNAUTHORIED);
        return;
    }

    this->setPermission(QSharedPointer<Permission>(new Permission{
        .read = rule->read,
        .write = rule->write,
        .execute = rule->execute,
    }));

    this->setState((rule->enable) ? DEVICE_STATE_ENABLE : DEVICE_STATE_DISABLE);
}

bool USBDevice::setEnable(bool enable)
{
    DeviceRule rule;

    rule.uid = m_uid;
    rule.id = this->getID();
    rule.name = this->getName();
    rule.idVendor = m_idVendor;
    rule.idProduct = m_idProduct;
    rule.enable = enable;
    rule.type = this->getType();
    rule.interfaceType = this->getInterfaceType();
    rule.read = this->getPermission()->read;
    rule.write = this->getPermission()->write;
    rule.execute = this->getPermission()->execute;

    m_ruleManager->addRule(rule);

    this->setState((rule.enable) ? DEVICE_STATE_ENABLE : DEVICE_STATE_DISABLE);
    
    return true;
}

void USBDevice::update()
{
    this->setType(this->parseDeviceType());
}

}  // namespace KS
