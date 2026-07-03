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

#include "src/daemon/dm/device-manager.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDateTime>
#include <QProcess>
#include "src/daemon/account/manager.h"
#include "src/daemon/common/dbus-helper.h"
#include "src/daemon/dm/dbus.h"
#include "src/daemon/dm/device-factory.h"
#include "src/daemon/dm/sd/sd-device-enumerator.h"
#include "src/daemon/dm/udev-rule-manager.h"
#include "src/daemon/log/manager.h"
#include "ssr-i.h"
#include "ssr-marcos.h"

namespace KS
{
namespace DM
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
    m_deviceDBus = new DBus(this, this);
    m_deviceLog = QSharedPointer<DeviceLog>::create();
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::init()
{
    m_deviceDBus->init();

    this->initDevices();

    UdevRuleManager::instance();

    connect(&m_sdDeviceMonitor, &SDDeviceMonitor::deviceChanged, this, &DeviceManager::handleUdevEvent);
    connect(&m_mountMonitor, &DeviceMountMonitor::mountChanged, this, &DeviceManager::handleMountEvent);
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

bool compareDeviceConnectedTime(const QSharedPointer<Device> deviceA,
                                const QSharedPointer<Device> deviceB)
{
    return deviceA->getConnectedTime() > deviceB->getConnectedTime();
}

DeviceList DeviceManager::getDevices()
{
    auto devices = this->m_devices.values();

    // 根据连接时间进行排序
    std::sort(devices.begin(), devices.end(), compareDeviceConnectedTime);

    return devices;
};

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

    // 根据连接时间进行排序
    std::sort(devices.begin(), devices.end(), compareDeviceConnectedTime);

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

QSharedPointer<Device> DeviceManager::getParentDevice(const QString syspath) const
{
    Q_FOREACH (auto device, m_devices)
    {
        auto path = device->getSyspath();
        if ((device->getType() != DEVICE_TYPE_HUB) &&
            syspath.startsWith(path.endsWith('/') ? path : path + '/'))
        {
            return device;
        }
    }

    return nullptr;
}

bool DeviceManager::isDeviceMountPerChanged(const QSharedPointer<Device> device,
                                            const DeviceMount *mount)
{
    auto permission = device->getPermission();

    return ((mount->write != permission->write) ||
            (mount->execute != permission->execute));
}

QString DeviceManager::getMountSyspath(const DeviceMount *mount)
{
    SDDeviceEnumerator enumerator;
    auto devices = enumerator.getDevices();

    Q_FOREACH (auto device, devices)
    {
        auto devname = device->getDevname();

        if (devname == mount->device)
        {
            return QString(device->getSyspath());
        }
    }

    return nullptr;
}

void DeviceManager::handleMountEvent(const DeviceMount *mount)
{
    auto mountSyspath = this->getMountSyspath(mount);
    RETURN_IF_TRUE(mountSyspath == nullptr)

    auto parentDevice = getParentDevice(mountSyspath);
    RETURN_IF_TRUE(parentDevice == nullptr)

    if (this->isDeviceMountPerChanged(parentDevice, mount))
    {
        this->remountDevice(parentDevice, mount);
    }
}

void DeviceManager::remountDevice(const QSharedPointer<Device> device,
                                  const DeviceMount *mount)
{
    auto permission = device->getPermission();
    QStringList args{"-o"};
    QString options("remount");
    options.append(permission->write ? ",rw" : ",ro");
    options.append(permission->execute ? "" : ",noexec");
    args.append({options, mount->device, mount->path});
    auto exitcode = QProcess::execute("mount", args);
    if (exitcode != 0)
    {
        KLOG_WARNING() << "Failed to execute command: "
                       << "mount " << args.join(' ') << ", exitcode is " << exitcode;
    }
}

void DeviceManager::checkDeviceMount(const QSharedPointer<Device> device)
{
    RETURN_IF_TRUE(device->getType() != DEVICE_TYPE_STORAGE)
    auto syspath = device->getSyspath();
    auto mounts = m_mountMonitor.getMounts();

    Q_FOREACH (auto mount, mounts)
    {
        auto mountSyspath = this->getMountSyspath(mount.get());
        if (mountSyspath == nullptr)
        {
            continue;
        }

        if (mountSyspath.startsWith(syspath) &&
            this->isDeviceMountPerChanged(device, mount.get()))
        {
            this->remountDevice(device, mount.get());
        }
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
    KS::Log::Log log{"sysadm", Account::Manager::AccountRole::sysadm,
                     QDateTime::currentDateTime(), Log::Manager::LogType::DEVICE, true,
                     tr("Device access, name is %1, type is %2")
                         .arg(record.name.isEmpty() ? tr("Unknown device") : record.name)
                         .arg(deviceTypeEnum2Str(record.type))};
    KS::Log::Manager::writeLog(log);
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
        device->update();
        Q_EMIT this->deviceChanged(device->getID(), DEVICE_ACTION_CHANGE);
    }
}

void DeviceManager::triggerInterfaceDevices(int interfaceType)
{
    auto type = interfaceType;

    if (interfaceType == INTERFACE_TYPE_USB_KBD ||
        interfaceType == INTERFACE_TYPE_USB_MOUSE)
    {
        type = INTERFACE_TYPE_USB;
    }

    for (auto device : m_devices)
    {
        // 重放此接口类型的设备的Udev事件
        if (device->getInterfaceType() == type)
        {
            device->trigger();
        }
    }
}

bool DeviceManager::isSupportHDMIDisable()
{
    for (auto device : m_devices)
    {
        // 重放此接口类型的设备的Udev事件
        if (device->getInterfaceType() == INTERFACE_TYPE_HDMI)
        {
            return true;
        }
    }

    return false;
}

QString DeviceManager::interfaceTypeEnum2Str(int type)
{
    switch (type)
    {
    case INTERFACE_TYPE_OTHER:
        return tr("Other");
    case INTERFACE_TYPE_USB:
        return tr("USB");
    case INTERFACE_TYPE_BLUETOOTH:
        return tr("Bluetooth");
    case INTERFACE_TYPE_NET:
        return tr("Network card");
    case INTERFACE_TYPE_HDMI:
        return tr("HDMI");
    case INTERFACE_TYPE_USB_KBD:
        return tr("Keyboard");
    case INTERFACE_TYPE_USB_MOUSE:
        return tr("Mouse");
    default:
        break;
    }
    return QString();
}

QString DeviceManager::deviceTypeEnum2Str(int type)
{
    switch (type)
    {
    case DEVICE_TYPE_STORAGE:
        return "Storage";
    case DEVICE_TYPE_CD:
        return "CD";
    case DEVICE_TYPE_MOUSE:
        return "Mouse";
    case DEVICE_TYPE_KEYBOARD:
        return "Keyboard";
    case DEVICE_TYPE_NET_CARD:
        return "Network card";
    case DEVICE_TYPE_WIRELESS_NET_CARD:
        return "Wireless network card";
    case DEVICE_TYPE_VIDEO:
        return "Video";
    case DEVICE_TYPE_AUDIO:
        return "Audio";
    case DEVICE_TYPE_PRINTER:
        return "Printer";
    case DEVICE_TYPE_HUB:
        return "Hub";
    case DEVICE_TYPE_COMMUNICATIONS:
        return "Communications";
    case DEVICE_TYPE_BLUETOOTH:
        return "Bluetooth";
    case DEVICE_TYPE_OTHER:
        return "Other";
    default:
        break;
    }
    return QString();
}

}  // namespace DM
}  // namespace KS
