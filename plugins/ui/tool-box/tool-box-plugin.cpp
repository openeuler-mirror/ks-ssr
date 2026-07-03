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

#include "tool-box-plugin.h"
#include "access-control/access-control-page.h"
#include "file-shred/file-shred-page.h"
#include "file-sign/file-sign-page.h"
#include "privacy-cleanup/privacy-cleanup-page.h"

namespace KS
{
namespace ToolBox
{
ToolBoxPlugin::ToolBoxPlugin()
{
    m_pageBuilder = {
        {QString("access-control"), []() -> Page*
         {
             return new AccessControlPage();
         }},
        {QString("file-shred"), []() -> Page*
         {
             return new FileShredPage();
         }},
        {QString("file-sign"), []() -> Page*
         {
             return new FileSign();
         }},
        {QString("privacy-cleanup"), []() -> Page*
         {
             return new PrivacyCleanupPage();
         }}};
}

Page* ToolBoxPlugin::createPage(const QString& pageUID)
{
    auto builder = m_pageBuilder.value(pageUID);
    if (builder)
    {
        return builder();
    }
    return nullptr;
}
}  // namespace ToolBox

}  // namespace KS
