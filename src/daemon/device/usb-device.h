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

#include "src/daemon/device/device-rule-manager.h"
#include "src/daemon/device/device.h"

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
    void initPermission();

private:
    QString m_idProduct;
    QString m_idVendor;
    QString m_manufacturer;
    QString m_product;
    QString m_uid;

    DeviceRuleManager *m_ruleManager;
};
}  // namespace KS