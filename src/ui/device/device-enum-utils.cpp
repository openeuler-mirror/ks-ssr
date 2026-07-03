#include "device-enum-utils.h"

#define ENABLE QObject::tr("Enable")
#define DISABLE QObject::tr("Disable")
#define UNAUTHORIED QObject::tr("Unauthoried")

namespace KS
{
QString DeviceEnumUtils::enum2Str(DeviceType enumVal)
{
    switch (enumVal)
    {
    case DEVICE_TYPE_DISK:
        return tr("Disk");
    case DEVICE_TYPE_CD:
        return tr("CD");
    case DEVICE_TYPE_MOUSE:
        return tr("Mouse");
    case DEVICE_TYPE_KEYBOARD:
        return tr("Keyboard");
    case DEVICE_TYPE_NET_CARD:
        return tr("Network card");
    case DEVICE_TYPE_WIRELESS_NET_CARD:
        return tr("Wireless network card");
    case DEVICE_TYPE_VIDEO:
        return tr("Video");
    case DEVICE_TYPE_AUDIO:
        return tr("Audio");
    case DEVICE_TYPE_PRINTER:
        return tr("Printer");
    case DEVICE_TYPE_HUB:
        return tr("Hub");
    case DEVICE_TYPE_COMMUNICATIONS:
        return tr("Communications");
    case DEVICE_TYPE_UNKNOWN:
        return tr("Unknown");
    default:
        break;
    }
    return QString();
}

QString DeviceEnumUtils::enum2Str(InterfaceType enumVal)
{
    switch (enumVal)
    {
    case INTERFACE_TYPE_USB:
        return tr("USB");
    case INTERFACE_TYPE_BLUETOOTH:
        return tr("Bluetooth");
    case INTERFACE_TYPE_NET:
        return tr("Network");
    case INTERFACE_TYPE_HDMI:
        return tr("HDMI");
    case INTERFACE_TYPE_UNKNOWN:
        return tr("Unknown");
    default:
        break;
    }
    return QString();
}

QString DeviceEnumUtils::enum2Str(DeviceState enumVal)
{
    switch (enumVal)
    {
    case DEVICE_STATE_ENABLE:
        return ENABLE;
    case DEVICE_STATE_DISABLE:
        return DISABLE;
    case DEVICE_STATE_UNAUTHORIED:
        return UNAUTHORIED;
    default:
        break;
    }
    return QString();
}

DeviceState DeviceEnumUtils::str2StateEnum(QString str)
{
    if (str == ENABLE)
        return DeviceState::DEVICE_STATE_ENABLE;
    else if (str == DISABLE)
        return DeviceState::DEVICE_STATE_DISABLE;
    else
        return DEVICE_STATE_UNAUTHORIED;
}
}  // namespace KS
