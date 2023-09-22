#include "device-utils.h"

namespace KS
{
QString DeviceUtils::deviceTypeEnum2Str(DeviceType type)
{
    switch (type)
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

QString DeviceUtils::interfaceTypeEnum2Str(InterfaceType type)
{
    switch (type)
    {
    case INTERFACE_TYPE_USB:
        return tr("USB");
    case INTERFACE_TYPE_BLUETOOTH:
        return tr("Bluetooth");
    case INTERFACE_TYPE_NET:
        return tr("Network");
    case INTERFACE_TYPE_HDMI:
        return tr("HDMI");
    case INTERFACE_TYPE_USB_MOUSE:
        return tr("Mouse");
    case INTERFACE_TYPE_USB_KBD:
        return tr("Keyboard");
    case INTERFACE_TYPE_UNKNOWN:
        return tr("Unknown");
    default:
        break;
    }
    return QString();
}

QString DeviceUtils::deviceStateEnum2Str(DeviceState state)
{
    switch (state)
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

QString DeviceUtils::deviceConnectStateEnum2Str(DeviceConnectState state)
{
    switch (state)
    {
    case DeviceConnectState::DEVICE_CONNECT_SUCCESSED:
        return SUCCESSFUL;
    case DeviceConnectState::DEVICE_CONNECT_FAILED:
        return FAILED;
    default:
        break;
    }
    return QString();
}

DeviceState DeviceUtils::deviceStateStr2Enum(const QString& state)
{
    if (state == ENABLE)
        return DeviceState::DEVICE_STATE_ENABLE;
    else if (state == DISABLE)
        return DeviceState::DEVICE_STATE_DISABLE;
    else
        return DEVICE_STATE_UNAUTHORIED;
}
}  // namespace KS
