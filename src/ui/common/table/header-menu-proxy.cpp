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

#include "header-menu-proxy.h"
#include <QMouseEvent>

namespace KS
{
HeaderMenuProxy::HeaderMenuProxy(QWidget *parent)
    : QMenu(parent)
{
}

void HeaderMenuProxy::mouseReleaseEvent(QMouseEvent *event)
{
    // 获取鼠标点击的action
    auto action = this->actionAt(event->pos());
    if (action)
    {
        // 触发action事件，不进行menu事件
        action->activate(QAction::Trigger);
    }
    else
    {
        // 若鼠标释放时没有点击action则触发默认menu事件，关闭菜单
        QMenu::mouseReleaseEvent(event);
    }
}
}  // namespace KS
