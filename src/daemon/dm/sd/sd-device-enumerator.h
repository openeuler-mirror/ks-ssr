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

#include <QObject>
#include "src/daemon/dm/sd/sd-device.h"

typedef struct sd_device_enumerator sd_device_enumerator;

namespace KS
{
namespace DM
{
class SDDeviceEnumerator : public QObject
{
    Q_OBJECT

public:
    SDDeviceEnumerator(QObject *parent = nullptr);
    virtual ~SDDeviceEnumerator();

    QList<SDDevice *> getDevices() const;

private:
    void init();

private:
    sd_device_enumerator *m_enumerator;
    QList<SDDevice *> m_devices;
};
}  // namespace DM
}  // namespace KS