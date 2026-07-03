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

#include <QWidget>

namespace KS
{
// 枚举值越小，在导航页面显示的位置越靠前
enum NavigationIndex
{
    // 远程管理
    REMOTE_MANAGEMENT,
    // 基线加固
    BASE_REINFORCEMENT,
    // 漏洞修复
    VULNERABILITY,
    // 可信保护
    TRUST_PROTECTION,
    // 文件保护
    FILE_PROTECTION,
    // 私密保险箱
    PRIVATE_SAFE_BOX,
    // 设备管理
    DEVICE_MANAGEMENT,
    // 安全工具箱
    SECURITY_TOOL_BOX,
    // 日志审计
    LOG_AUDIT,
    // 分类数量
    COUNT
};

class WorkPage : public QWidget
{
    Q_OBJECT
public:
    WorkPage(QWidget* parent = nullptr)
        : QWidget(parent){};

    virtual ~WorkPage(){};

    // 判断页面是否已经初始化完毕，如果没初始化完毕应该显示一个加载动画页面
    virtual bool isInitialized()
    {
        return true;
    }

    virtual NavigationIndex getNavigationIndex() = 0;
    virtual QString getSidebarUID() = 0;
    virtual QString getSidebarIcon() = 0;
    // TODO: 后面删除掉，放到配置文件中
    virtual QString getAccountRoleName() = 0;

Q_SIGNALS:
    void initFinished();
};

class SettingPage : public QWidget
{
    Q_OBJECT
public:
    SettingPage(QWidget* parent = nullptr)
        : QWidget(parent){};
    virtual ~SettingPage(){};

    virtual QString getTitle() = 0;
};

}  // namespace KS
