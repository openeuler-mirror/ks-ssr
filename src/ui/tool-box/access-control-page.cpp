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

#include "access-control-page.h"
#include "include/ssr-i.h"

#define ACCESS_CONTROL_ICON_NAME "/images/access-control"

namespace KS
{
namespace ToolBox
{
AccessControl::AccessControl(QWidget* parent)
    : Page(parent)
{
}

AccessControl::~AccessControl()
{
}

QString AccessControl::getNavigationUID()
{
    return tr("Tool Box");
}

QString AccessControl::getSidebarUID()
{
    return tr("Access Control");
}

QString AccessControl::getSidebarIcon()
{
    return ":" ACCESS_CONTROL_ICON_NAME;
}

int AccessControl::getSelinuxType()
{
    return 0;
}
}  // namespace ToolBox
}  // namespace KS