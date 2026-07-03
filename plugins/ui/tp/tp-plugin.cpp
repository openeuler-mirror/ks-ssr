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

#include "tp-plugin.h"
#include "execute-protected-page.h"
#include "kernel-protected-page.h"
#include "tp-setting-page.h"

namespace KS
{
namespace TP
{
TPPlugin::TPPlugin()
{
    m_workPageBuilder = {
        {QString("execute-protection"), []() -> WorkPage*
         {
             return new ExecuteProtectedPage();
         }},
        {QString("kernel-protection"), []() -> WorkPage*
         {
             return new KernelProtectedPage();
         }}};

    m_settingPageBuilder = {
        {QString("tp-setting"), []() -> SettingPage*
         {
             return new TPSettingPage();
         }}};
}

WorkPage* TPPlugin::createWorkPage(const QString& pageUID)
{
    auto builder = m_workPageBuilder.value(pageUID);
    if (builder)
    {
        return builder();
    }
    return nullptr;
}

SettingPage* TPPlugin::createSettingPage(const QString& pageUID)
{
    auto builder = m_settingPageBuilder.value(pageUID);
    if (builder)
    {
        return builder();
    }
    return nullptr;
}
}  // namespace TP

}  // namespace KS
