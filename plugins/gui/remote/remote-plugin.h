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

#pragma once

#include <ui-plugin-i.h>
#include <QCoreApplication>
#include "lib/base/misc-utils.h"
#include "remote-page.h"

class QTranslator;

namespace KS
{
namespace Remote
{
class RemotePlugin : public UIPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IUI_IID FILE "remote-plugin.json")
    Q_INTERFACES(KS::IUIPlugin)

public:
    RemotePlugin()
    {
        m_workPageBuilder = {
            {QString("remote-management"), []() -> WorkPage*
             {
                 return new RemotePage();
             }}};

        m_translator = MiscUtils::installTranslator(QString("%1-%2").arg(QCoreApplication::applicationName()).arg("remote"));
    }

    virtual ~RemotePlugin()
    {
        MiscUtils::removeTranslator(m_translator);
    }

private:
    QTranslator* m_translator;
};

}  // namespace Remote
}  // namespace KS
