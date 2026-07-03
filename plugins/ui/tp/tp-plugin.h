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
#include <QMap>
#include <QObject>
#include <QString>
#include <functional>

namespace KS
{
namespace TP
{
class TPPlugin : public QObject,
                 public IUIPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IUI_IID FILE "tp-plugin.json")
    Q_INTERFACES(KS::IUIPlugin)

public:
    TPPlugin();

    // 创建页面
    virtual WorkPage* createWorkPage(const QString& pageUID);
    virtual SettingPage* createSettingPage(const QString& pageUID);

private:
    // <pageUID, 创建page对象的函数>
    QMap<QString, std::function<WorkPage*()>> m_workPageBuilder;
    QMap<QString, std::function<SettingPage*()>> m_settingPageBuilder;
};

}  // namespace TP
}  // namespace KS
