#include "header-menu-proxy.h"
#include <QMouseEvent>

namespace KS
{
HeaderMenuProxy::HeaderMenuProxy(QWidget *parent) : QMenu(parent)
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
