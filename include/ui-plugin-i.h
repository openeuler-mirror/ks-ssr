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

#include <QMap>
#include <QObject>
#include <QString>
#include <QtPlugin>
#include <functional>

namespace KS
{
#define IUI_IID "com.kylinsec.ssr.ui.plugin"

class WorkPage;
class SettingPage;

class IUIPlugin
{
public:
    virtual ~IUIPlugin(){};

    // 创建主窗口的工作页面
    virtual WorkPage* createWorkPage(const QString& pageUID) = 0;
    // 创建设置页面
    virtual SettingPage* createSettingPage(const QString& pageUID) = 0;
};

class UIPlugin : public QObject,
                 public IUIPlugin
{
    Q_OBJECT

public:
    virtual ~UIPlugin() {}

    virtual WorkPage* createWorkPage(const QString& pageUID)
    {
        auto builder = m_workPageBuilder.value(pageUID);
        if (builder)
        {
            return builder();
        }
        return nullptr;
    }
    virtual SettingPage* createSettingPage(const QString& pageUID)
    {
        auto builder = m_settingPageBuilder.value(pageUID);
        if (builder)
        {
            return builder();
        }
        return nullptr;
    }

protected:
    // <pageUID, 创建page对象的函数>
    QMap<QString, std::function<WorkPage*()>> m_workPageBuilder;
    QMap<QString, std::function<SettingPage*()>> m_settingPageBuilder;
};

}  // namespace KS

Q_DECLARE_INTERFACE(KS::IUIPlugin, IUI_IID)
