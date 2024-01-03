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

#include "src/daemon/dm/device.h"
#include <qt5-log-i.h>
#include <QDateTime>
#include "ssr-i.h"
#include "ssr-marcos.h"

namespace KS
{
namespace DM
{
Device::Device(const QString& syspath, QObject* parent)
    : QObject(parent),
      m_type(DeviceType::DEVICE_TYPE_OTHER),
      m_interfaceType(InterfaceType::INTERFACE_TYPE_UNKNOWN),
      m_state(DeviceState::DEVICE_STATE_UNAUTHORIED)
{
    m_permission = QSharedPointer<Permission>(new Permission{
        .read = false,
        .write = false,
        .execute = false,
    });

    m_device = QSharedPointer<SDDevice>(new SDDevice(syspath));
    m_syspath = syspath;

    m_connectedTime = QDateTime::currentMSecsSinceEpoch();
}

Device::~Device() {}

bool Device::setEnable(bool enable)
{
    return false;
}

void Device::update()
{
}

QString Device::getID() const
{
    return m_id;
}

void Device::setID(QString& id)
{
    m_id = id;
}

QString Device::getName() const
{
    return m_name;
}

void Device::setName(QString& name)
{
    m_name = name;
}

QString Device::getSyspath() const
{
    return m_syspath;
}

int Device::getType() const
{
    return m_type;
}

void Device::setType(int type)
{
    m_type = type;
}

int Device::getInterfaceType() const
{
    return m_interfaceType;
}

void Device::setInterfaceType(int type)
{
    m_interfaceType = type;
}

int Device::getState() const
{
    return m_state;
}

void Device::setState(int state)
{
    m_state = state;
}

QSharedPointer<Permission> Device::getPermission() const
{
    return m_permission;
}

void Device::setPermission(QSharedPointer<Permission> permission)
{
    m_permission = permission;
}

void Device::trigger()
{
    m_device->trigger();
}

QSharedPointer<SDDevice> Device::getSDDevcie()
{
    return m_device;
}

QJsonObject Device::toJsonObject()
{
    auto permission = this->getPermission();

    QJsonObject jsonObj{
        {SSR_DEVICE_JK_ID, this->getID()},
        {SSR_DEVICE_JK_NAME, this->getName()},
        {SSR_DEVICE_JK_TYPE, this->getType()},
        {SSR_DEVICE_JK_INTERFACE_TYPE, this->getInterfaceType()},
        {SSR_DEVICE_JK_READ, permission->read},
        {SSR_DEVICE_JK_WRITE, permission->write},
        {SSR_DEVICE_JK_EXECUTE, permission->execute},
        {SSR_DEVICE_JK_STATE, this->getState()}};

    return jsonObj;
}

qint64 Device::getConnectedTime()
{
    return m_connectedTime;
}
}  // namespace DM
}  // namespace KS
