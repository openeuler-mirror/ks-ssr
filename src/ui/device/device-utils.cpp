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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */
#include "device-utils.h"

#define ENABLE QObject::tr("Enable")
#define DISABLE QObject::tr("Disable")
#define UNAUTHORIED QObject::tr("Unauthoried")

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
        return tr("Successful");
    case DeviceConnectState::DEVICE_CONNECT_FAILED:
        return tr("Failed");
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
