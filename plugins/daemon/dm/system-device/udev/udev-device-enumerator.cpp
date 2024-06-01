#include "udev-device-enumerator.h"
#include <libudev.h>
#include <qt5-log-i.h>
#include <QList>
#include "udev-device.h"

struct udev* udev = udev_new();

namespace KS
{
namespace DM
{
UdevDeviceEnumerator::UdevDeviceEnumerator()
{
    m_deviceEnum = udev_enumerate_new(udev);
    if (udev_enumerate_scan_devices(m_deviceEnum))
    {
        KLOG_ERROR() << "Failed to init device enumerator";
    }
    auto udevEntry = udev_enumerate_get_list_entry(m_deviceEnum);
    while (udevEntry)
    {
        auto device = udev_device_new_from_syspath(udev, udev_list_entry_get_name(udevEntry));
        m_devices.append(new UdevDevice{device});
        // 因为构造了 UdevDevice 会使 device 的引用计数加一， 所以这里需要 unref 一下。
        udev_device_unref(device);
        udevEntry = udev_list_entry_get_next(udevEntry);
    }
}
UdevDeviceEnumerator::~UdevDeviceEnumerator()
{
    udev_enumerate_unref(m_deviceEnum);
    for (auto device : m_devices)
    {
        delete device;
    }
}

QList<UdevDevice*> UdevDeviceEnumerator::getDevices() const
{
    return m_devices;
}

}  // namespace DM
}  // namespace KS
