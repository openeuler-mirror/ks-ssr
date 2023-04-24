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

#pragma once

#include <QObject>
#include <QSharedPointer>
#include "src/daemon/device/sd-device.h"

namespace KS
{
struct Permission
{
public:
    Permission() = default;
    bool read;
    bool write;
    bool execute;

    Permission &operator=(const Permission &arg)
    {
        this->read = arg.read;
        this->write = arg.write;
        this->execute = arg.execute;

        return *this;
    }
};

class Device : public QObject
{
    Q_OBJECT

public:
    Device(const QString &syspath, QObject *parent = nullptr);
    virtual ~Device();
    virtual int setEnable(bool enable);
    virtual void update();
    QString getId() const;
    void setId(QString &id);
    QString getName() const;
    void setName(QString &name);
    QString getSyspath() const;
    int getType() const;
    void setType(int type);
    int getInterfaceType() const;
    void setInterfaceType(int type);
    int getState() const;
    void setState(int state);
    QSharedPointer<Permission> getPermission() const;
    void setPermission(const QString &rule);
    void setPermission(QSharedPointer<Permission> permission);
    QString getPermissionMode();
    void trigger();
    QSharedPointer<SdDevice> getSdDevcie();

private:
    void setPermission(int permission);

private:
    QString m_id;
    QString m_name;
    QString m_syspath;
    int m_type;
    int m_interfaceType;
    int m_state;
    QSharedPointer<Permission> m_permission;
    QSharedPointer<SdDevice> m_device;
};
}  // namespace KS