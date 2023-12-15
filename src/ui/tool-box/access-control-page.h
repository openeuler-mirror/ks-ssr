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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#pragma once

#include "src/ui/common/page.h"

namespace KS
{
namespace ToolBox
{
class AccessControl : public Page
{
    Q_OBJECT
public:
    AccessControl(QWidget *parent = nullptr);
    virtual ~AccessControl();
    virtual QString getNavigationUID() override;
    virtual QString getSidebarUID() override;
    virtual QString getSidebarIcon() override;
    virtual QString getAccountRoleName() override;
};
}  // namespace ToolBox
}  // namespace KS