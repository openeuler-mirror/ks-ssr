/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     chendingjian <chendingjian@kylinos.com.cn>
 */

#pragma once

#include "src/ui/common/page.h"

namespace Ui
{
class AccessControlPage;
}  // namespace Ui

namespace KS
{
namespace ToolBox
{
class AccessControlPage : public Page
{
    Q_OBJECT
public:
    AccessControlPage(QWidget* parent = nullptr);
    virtual ~AccessControlPage();
    virtual QString getNavigationUID() override;
    virtual QString getSidebarUID() override;
    virtual QString getSidebarIcon() override;
    virtual QString getAccountRoleName() override;

private:
    void initUI();

private:
    Ui::AccessControlPage* m_ui;
};
}  // namespace ToolBox
}  // namespace KS
