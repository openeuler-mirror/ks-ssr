#include "udev-device.h"
#include <libudev.h>
#include <QFile>
#include "ssr-marcos.h"

extern struct udev* udev;

namespace KS
{
namespace DM
{
UdevDevice::UdevDevice(udev_device* device)
{
    m_device = udev_device_ref(device);
}

UdevDevice::UdevDevice(const UdevDevice& device)
{
    m_device = udev_device_ref(device.m_device);
}

UdevDevice::UdevDevice(const QString& syspath)
{
#pragma message("todo")
    m_device = udev_device_new_from_syspath(udev, syspath.toLatin1().data());
}

UdevDevice UdevDevice::operator=(const UdevDevice& device)
{
    return UdevDevice(device);
}

UdevDevice::~UdevDevice()
{
    udev_device_unref(m_device);
}

QString UdevDevice::getSyspath() const
{
    return QString(udev_device_get_syspath(m_device));
}

QString UdevDevice::getSubsystem() const
{
    return QString(udev_device_get_subsystem(m_device));
}

QString UdevDevice::getDevtype() const
{
    return QString(udev_device_get_devtype(m_device));
}

QString UdevDevice::getDevname() const
{
    return getDevNode();
}

QString UdevDevice::getSysname() const
{
    return QString(udev_device_get_sysname(m_device));
}

QString UdevDevice::getSysattrValue(const QString& attrName) const
{
    RETURN_VAL_IF_TRUE(attrName.isNull(), QString());

    return QString(udev_device_get_sysattr_value(m_device, attrName.toLatin1().data()));
}

QString UdevDevice::getDevNode() const
{
    return QString(udev_device_get_devnode(m_device));
}

void UdevDevice::trigger()
{
    auto filename = QString("%1/uevent").arg(this->getSyspath());
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "Cannot open file " << filename;
        return;
    }

    QTextStream out(&file);
    out << "change";

    file.close();
}

}  // namespace DM
}  // namespace KS