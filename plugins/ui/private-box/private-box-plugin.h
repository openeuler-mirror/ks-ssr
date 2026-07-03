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

#pragma once

#include <ui-plugin-i.h>
#include "box-page.h"

namespace KS
{
namespace PrivateBox
{
class PrivateBoxPlugin : public UIPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IUI_IID FILE "private-box-plugin.json")
    Q_INTERFACES(KS::IUIPlugin)

public:
    PrivateBoxPlugin()
    {
        m_workPageBuilder = {
            {QString("private-box"), []() -> WorkPage*
             {
                 return new BoxPage();
             }}};
    }
};

}  // namespace PrivateBox
}  // namespace KS
