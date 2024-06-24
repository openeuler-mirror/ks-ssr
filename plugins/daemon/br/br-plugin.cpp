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
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#include "br-plugin.h"
#include <QCoreApplication>
#include "context.h"
#include "lib/base/misc-utils.h"

namespace KS
{
namespace BR
{
BRPlugin::BRPlugin()
    : m_translator(nullptr)
{
}

bool BRPlugin::isAvailable()
{
    return true;
}

void BRPlugin::activate()
{
    m_translator = MiscUtils::installTranslator(QString("%1-%2").arg(QCoreApplication::applicationName()).arg("br"));
    Configuration::globalInit(SSR_INSTALL_DATADIR "/ssr.ini");
    Categories::globalInit();
    Plugins::globalInit(Configuration::getInstance());
    BRDBus::globalInit(nullptr);
}

void BRPlugin::deactivate()
{
    BRDBus::globalDeinit();
    Plugins::globalDeinit();
    Categories::globalDeinit();
    Configuration::globalDeinit();
    MiscUtils::removeTranslator(m_translator);
}
}  // namespace BR

}  // namespace KS
