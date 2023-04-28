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

#include "src/daemon/device/device.h"
#include <qt5-log-i.h>
#include "ksc-marcos.h"

namespace KS
{
#define MAX_PERMISSION_VALUE 7

#define PERMISSION_VALUE_READ 3     // 二进制100
#define PERMISSION_VALUE_WRITE 2    // 二进制010
#define PERMISSION_VALUE_EXECUTE 1  // 二进制001

#define PERMISSION_BIN_VALUE_READ 4     // 2的2次方
#define PERMISSION_BIN_VALUE_WRITE 2    // 2的1次方
#define PERMISSION_BIN_VALUE_EXECUTE 1  // 2的0次方

Device::Device(const QString& syspath, QObject* parent)
    : QObject(parent)
{
    m_permission = QSharedPointer<Permission>(new Permission{
        .read = false,
        .write = false,
        .execute = false,
    });

    m_device = QSharedPointer<SdDevice>(new SdDevice(syspath));
    m_syspath = syspath;
}

Device::~Device() {}

int Device::setEnable(bool enable)
{
    return false;
}

void Device::update()
{
    m_device = QSharedPointer<SdDevice>(new SdDevice(m_syspath));
}

QString Device::getId() const
{
    return m_id;
}

void Device::setId(QString& id)
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

void Device::setPermission(const QString& rule)
{
    auto mode = rule.section("MODE=", 1, 1);

    RETURN_IF_TRUE(mode.isNull());

    auto permission = mode.mid(2, 1);
    RETURN_IF_TRUE(permission.isNull());

    this->setPermission(permission.toInt());
}

void Device::setPermission(QSharedPointer<Permission> permission)
{
    m_permission = permission;
}

void Device::setPermission(int permission)
{
    RETURN_IF_TRUE(permission > MAX_PERMISSION_VALUE)

    m_permission->read = permission & PERMISSION_VALUE_READ;
    m_permission->write = permission & PERMISSION_VALUE_WRITE;
    m_permission->execute = permission & PERMISSION_VALUE_EXECUTE;
}

QString Device::getPermissionMode()
{
    auto permission = m_permission->read * PERMISSION_BIN_VALUE_READ +
                 m_permission->write * PERMISSION_BIN_VALUE_WRITE +
                 m_permission->execute * PERMISSION_BIN_VALUE_EXECUTE;

    return QString::asprintf("0%d%d%d", permission, permission, permission);
}

void Device::trigger()
{
    m_device->trigger();
}

QSharedPointer<SdDevice> Device::getSdDevcie()
{
    return m_device;
}
}  // namespace KS
