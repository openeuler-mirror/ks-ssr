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
#include "access-control/access-control-page.h"
#include "authentication-setting-page.h"
#include "file-shred/file-shred-page.h"
#include "file-sign/file-sign-page.h"
#include "privacy-cleanup/privacy-cleanup-page.h"

namespace KS
{
namespace ToolBox
{
class ToolBoxPlugin : public UIPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IUI_IID FILE "tool-box-plugin.json")
    Q_INTERFACES(KS::IUIPlugin)

public:
    ToolBoxPlugin()
    {
        m_workPageBuilder = {
            {QString("access-control"), []() -> WorkPage*
             {
                 return new AccessControlPage();
             }},
            {QString("file-shred"), []() -> WorkPage*
             {
                 return new FileShredPage();
             }},
            {QString("file-sign"), []() -> WorkPage*
             {
                 return new FileSign();
             }},
            {QString("privacy-cleanup"), []() -> WorkPage*
             {
                 return new PrivacyCleanupPage();
             }}};

        m_settingPageBuilder = {
            {QString("authentication-setting"), []() -> SettingPage*
             {
                 return new AuthenticationSettingPage();
             }}};
    }
};

}  // namespace ToolBox
}  // namespace KS
