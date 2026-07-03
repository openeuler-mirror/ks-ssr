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

#include "src/daemon/dm/drm-device.h"
#include <ssr-i.h>
#include <QFileInfo>

namespace KS
{
DRMDevice::DRMDevice(const QString &syspath, QObject *parent) : Device(syspath, parent)
{
    this->init();
}

void DRMDevice::init()
{
    auto sdDevice = this->getSDDevcie();

    auto syspath = sdDevice->getSyspath();
    auto syspathBaseName = QFileInfo(syspath).baseName();
    if (syspathBaseName.contains("HDMI"))
    {
        this->setInterfaceType(INTERFACE_TYPE_HDMI);
    }
}
}  // namespace KS
