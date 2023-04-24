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

#include <systemd/sd-device.h>
#include <QObject>

namespace KS
{

enum UdevAction
{
    SD_DEVICE_ACTION_INVALID = 0,
    SD_DEVICE_ACTION_ADD,
    SD_DEVICE_ACTION_REMOVE,
    SD_DEVICE_ACTION_CHANGE
};

class SdDevice : public QObject
{
    Q_OBJECT

public:
    SdDevice(sd_device *device, QObject *parent = nullptr);
    SdDevice(const QString &syspath, QObject *parent = nullptr);
    virtual ~SdDevice();
    QString get_syspath() const;
    QString get_subsystem() const;
    QString get_devtype() const;
    QString get_sysname() const;
    QString get_sysattr_value(const QString &attr) const;
    int get_action() const;
    void trigger();

private:
    sd_device *m_device;
};

}  // namespace KS