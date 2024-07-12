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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#include "include/ssr-i.h"
#ifdef WITH_SYSTEMD_UDEV
#include "systemd-udev/sd-device-monitor.h"
#else
#include "udev/udev-device-monitor.h"
#endif

namespace KS
{
namespace DM
{
using SystemDeviceMonitor
#ifdef WITH_SYSTEMD_UDEV
    = SDDeviceMonitor;
#else
    = UdevDeviceMonitor;
#endif
}  // namespace DM
}  // namespace KS