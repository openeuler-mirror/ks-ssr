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

#include "src/daemon/device/device-manager.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "src/daemon/device/device-dbus.h"
#include "src/daemon/device/device-factory.h"
#include "src/daemon/device/sd/sd-device-enumerator.h"

namespace KS
{
DeviceManager *DeviceManager::m_instance = nullptr;
void DeviceManager::globalInit(QObject *parent)
{
    m_instance = new DeviceManager(parent);
    m_instance->init();
}

void DeviceManager::globalDeinit()
{
    if (m_instance)
    {
        delete m_instance;
    }
}

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
    m_deviceFactory = new DeviceFactory(this);
    m_deviceDBus = new DeviceDBus(this, this);
    m_deviceLog = QSharedPointer<DeviceLog>::create();
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::init()
{
    m_deviceDBus->init();

    this->initDevices();

    connect(&m_sdDeviceMonitor, &SDDeviceMonitor::deviceChanged, this, &DeviceManager::handleUdevEvent);
}

void DeviceManager::initDevices()
{
    SDDeviceEnumerator enumerator;
    auto devices = enumerator.getDevices();

    Q_FOREACH (auto device, devices)
    {
        this->addDevice(device);
    }
}

DeviceList DeviceManager::getDevicesByInterface(int interfaceType)
{
    DeviceList devices;
    for (auto device : m_devices)
    {
        if (device->getInterfaceType() == interfaceType)
        {
            devices.push_back(device);
        }
    }
    return devices;
}

QSharedPointer<Device> DeviceManager::getDeviceByID(const QString &id)
{
    for (auto device : m_devices)
    {
        if (device->getID() == id)
        {
            return device;
        }
    }
    return QSharedPointer<Device>();
}

void DeviceManager::addDevice(SDDevice *sdDevice)
{
    auto syspath = sdDevice->getSyspath();

    RETURN_IF_TRUE(syspath.isNull())

    auto device = m_deviceFactory->createDevice(sdDevice);

    if (device)
    {
        m_devices.insert(device->getSyspath(), device);
        KLOG_INFO() << "Device added with syspath " << device->getSyspath();
    }
}

void DeviceManager::handleUdevEvent(SDDevice *device,
                                    int action)
{
    switch (action)
    {
    case SD_DEVICE_ACTION_REMOVE:
        this->handleUdevRemoveEvent(device);
        break;

    case SD_DEVICE_ACTION_ADD:
        this->handleUdevAddEvent(device);
        break;

    case SD_DEVICE_ACTION_CHANGE:
        this->handleUdevChangeEvent(device);
        break;

    default:
        break;
    }
}

void DeviceManager::recordDeviceConnection(QSharedPointer<Device> device)
{
    DeviceRecord record;

    record.name = device->getName();
    record.type = device->getType();

    record.state = device->getState() == DEVICE_STATE_ENABLE ? DEVICE_CONNECT_SUCCESSED : DEVICE_CONNECT_FAILED;

    // 以秒为单位的时间戳
    record.time = QDateTime::currentSecsSinceEpoch();

    m_deviceLog->addDeviceRecord(record);
}

void DeviceManager::handleUdevAddEvent(SDDevice *sdDevice)
{
    auto syspath = sdDevice->getSyspath();

    RETURN_IF_TRUE(m_devices.contains(syspath));

    this->addDevice(sdDevice);

    auto device = m_devices.value(syspath);
    if (device)
    {
        this->recordDeviceConnection(device);
        Q_EMIT this->deviceChanged(device->getID(), DEVICE_ACTION_ADD);
    }
}

void DeviceManager::handleUdevRemoveEvent(SDDevice *sdDevice)
{
    auto syspath = sdDevice->getSyspath();
    auto device = m_devices.take(syspath);

    if (device)
    {
        KLOG_INFO() << "Device removed with syspath " << syspath;
        Q_EMIT this->deviceChanged(device->getID(), DEVICE_ACTION_REMOVE);
    }
}

void DeviceManager::handleUdevChangeEvent(SDDevice *sdDevice)
{
    auto syspath = sdDevice->getSyspath();
    auto device = m_devices.value(syspath);

    if (device)
    {
        KLOG_INFO() << "Device changed with syspath " << syspath;
        Q_EMIT this->deviceChanged(device->getID(), DEVICE_ACTION_CHANGE);
    }
}

}  // namespace KS
