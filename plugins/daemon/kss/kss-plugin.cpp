/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "kss-plugin.h"
#include <QFile>
#include "config.h"
#include "dbus.h"

namespace KS
{
namespace KSS
{
#define KSS_CMD_PATH SSR_INSTALL_BINDIR "/kss"

bool KSSPlugin::isAvailable()
{
    return QFile::exists(KSS_CMD_PATH);
}

void KSSPlugin::activate()
{
    DBus::globalInit(this);
}

void KSSPlugin::deactivate()
{
    DBus::globalDeinit();
}
}  // namespace KSS

}  // namespace KS
