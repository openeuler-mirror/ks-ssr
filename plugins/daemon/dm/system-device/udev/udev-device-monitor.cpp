#include "udev-device-monitor.h"
#include <libudev.h>
#include <qt5-log-i.h>
#include <QSocketNotifier>
#include "include/ssr-i.h"
#include "udev-device-enumerator.h"
#include "udev-device.h"

extern struct udev* udev;

namespace KS
{
namespace DM
{
UdevDeviceMonitor::UdevDeviceMonitor()
{
    initDevices();
    m_udevDeviceMonitor = udev_monitor_new_from_netlink(udev, "udev");
    if (udev_monitor_enable_receiving(m_udevDeviceMonitor))
    {
        KLOG_ERROR() << "failed to init device monitor!";
    }
    m_deviceNotify = new QSocketNotifier(udev_monitor_get_fd(m_udevDeviceMonitor), QSocketNotifier::Read);
    QObject::connect(m_deviceNotify, &QSocketNotifier::activated, this, &UdevDeviceMonitor::onUdevDeviceChanged);
}
UdevDeviceMonitor::~UdevDeviceMonitor()
{
    delete m_deviceNotify;
    udev_monitor_unref(m_udevDeviceMonitor);
}

bool UdevDeviceMonitor::isDeviceExisted(const QString& syspath) const
{
    UdevDeviceEnumerator devEnumerator{};
    const auto devices = devEnumerator.getDevices();
    for (const auto device : devices)
    {
        if (device->getSyspath() == syspath)
        {
            return true;
        }
    }
    return false;
}

void UdevDeviceMonitor::onUdevDeviceChanged(int)
{
    auto device = udev_monitor_receive_device(m_udevDeviceMonitor);
    UdevDevice udevDevice{device};
    const QString& syspath = udevDevice.getSyspath();
    if (this->isDeviceExisted(syspath))
    {
        if (!m_devices.value(syspath))
        {
            m_devices.insert(syspath, QSharedPointer<UdevDevice>(new UdevDevice(syspath)));
            KLOG_INFO() << "Action: Add";
            Q_EMIT this->deviceChanged(&udevDevice, DEVICE_ACTION_ADD);
        }
        else
        {
            KLOG_INFO() << "Action: Change";
            Q_EMIT this->deviceChanged(&udevDevice, DEVICE_ACTION_CHANGE);
        }
    }
    else
    {
        KLOG_INFO() << "Action: Remove";
        Q_EMIT this->deviceChanged(&udevDevice, DEVICE_ACTION_REMOVE);
        m_devices.remove(syspath);
    }
    udev_device_unref(device);
}

void UdevDeviceMonitor::initDevices()
{
    UdevDeviceEnumerator devEnumerator{};
    const auto devices = devEnumerator.getDevices();

    Q_FOREACH (auto device, devices)
    {
        QString syspath = device->getSyspath();

        if (syspath.isNull())
        {
            continue;
        }

        m_devices.insert(syspath, QSharedPointer<UdevDevice>(new UdevDevice(syspath)));
    }
}

}  // namespace DM
}  // namespace KS