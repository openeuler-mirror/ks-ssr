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
 * Author:     wangxiaoqing <wangxiaoqing@kylinsec.com.cn>
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QSharedPointer>
#include "system-device/system-device.h"

namespace KS
{
namespace DM
{
struct Permission
{
    bool read;
    bool write;
    bool execute;
};

class Device : public QObject
{
    Q_OBJECT

public:
    Device(const QString &syspath, QObject *parent = nullptr);
    virtual ~Device();

public:
    virtual bool setEnable(bool enable);
    virtual void update();

public:
    QString getID() const;
    QString getName() const;
    int getType() const;
    int getInterfaceType() const;
    int getState() const;
    QString getSyspath() const;
    QSharedPointer<Permission> getPermission() const;
    QSharedPointer<SystemDevice> getDevcie();
    qint64 getConnectedTime();

    void setID(QString &id);
    void setName(QString &name);
    void setType(int type);
    void setInterfaceType(int type);
    void setState(int state);
    void setPermission(QSharedPointer<Permission> permission);

public:
    QJsonObject toJsonObject();
    void trigger();

private:
    QString m_id;
    QString m_name;
    QString m_syspath;
    int m_type;
    int m_interfaceType;
    int m_state;
    QSharedPointer<Permission> m_permission;
    QSharedPointer<SystemDevice> m_device;

    qint64 m_connectedTime;
};

using DeviceList = QList<QSharedPointer<Device>>;
}  // namespace DM
}  // namespace KS