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

#pragma once

#include "src/daemon/dm/configuration.h"
#include "src/daemon/dm/device.h"

namespace KS
{
struct InterfaceClass
{
public:
    InterfaceClass() = default;
    int bInterfaceClass;
    int bInterfaceSubClass;
    int bInterfaceProtocol;
};

class USBDevice : public Device
{
    Q_OBJECT

public:
    USBDevice(const QString &syspath, QObject *parent = nullptr);
    virtual ~USBDevice();
    virtual bool setEnable(bool enable);
    virtual void update();

private:
    void init();
    int parseDeviceType();
    int deviceClass2DeviceType();
    int parseDeviceInterfaceClassType();
    int interfaceProtocol2DevcieType(const InterfaceClass &interface);
    int hidProtocol2DevcieType(const InterfaceClass &interface);
    // wireless controller
    int wcProtocol2DevcieType(const InterfaceClass &interface);
    void initPermission();
    bool isEnable();
    void setDeviceAuthorized();

private:
    QString m_idProduct;
    QString m_idVendor;
    QString m_manufacturer;
    QString m_product;
    QString m_uid;

    Configuration *m_devConfig;

    /* 维护一个映射表，
    key: QString "${idVendor}:${idProduct}"
    value: int 硬件种类
    用于指定那些厂商没有正确设置硬件中类的硬件信息。*/
    static QMap<QString, int> m_fixedTypes;
};
}  // namespace KS