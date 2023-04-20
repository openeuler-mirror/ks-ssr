/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * ks-sc is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#include "titlebar-window.h"
#include "global-define.h"
#include "src/ui/common/xlib-helper.h"
#include "titlebar-window-private.h"

#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QWindow>

namespace KS
{
TitlebarWindow::TitlebarWindow(QWidget *parent,
                               Qt::WindowFlags windowFlags) : QWidget(parent),
                                                              d_ptr(new TitlebarWindowPrivate(this))
{
    auto flags = Qt::WindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setWindowFlags(flags);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_Hover);

    d_func()->init();
    QWidget::ensurePolished();
    setTitle(qApp->applicationName());
}

TitlebarWindow::~TitlebarWindow()
{
    delete d_ptr;
}

void TitlebarWindow::setWindowContentWidget(QWidget *widget)
{
    d_func()->setWindowContentWidget(widget);
}

void TitlebarWindow::setIcon(const QIcon &icon)
{
    setWindowIcon(icon);
    d_func()->setIcon(icon);
}

void TitlebarWindow::setTitle(const QString &text)
{
    setWindowTitle(text);
    d_func()->setTitle(text);
    /* NOTE:由于标题栏标题文本需要收缩以获取更大的大小分配，所以保存完整的标题文本，用作每次省略压缩显示的参考 */
    d_func()->m_titleBarLayout->setTitleBarCompleteTitle(text);
}

void TitlebarWindow::setButtonHints(TitlebarButtonHintFlags hints)
{
    d_func()->setButtonHints(hints);
}

void TitlebarWindow::setResizeable(bool resizeable)
{
    d_func()->m_resizeable = resizeable;
}

QWidget *TitlebarWindow::getWindowContentWidget()
{
    return d_func()->m_windowContentWidget;
}

QHBoxLayout *TitlebarWindow::getTitlebarCustomLayout()
{
    return d_func()->m_customLayout;
}

bool TitlebarWindow::event(QEvent *event)
{
    return QWidget::event(event);
}

bool TitlebarWindow::titlebarCustomLayoutAlignHCenter()
{
    return d_ptr->m_titleBarLayout->customWidgetCenter();
}

void TitlebarWindow::setTitlebarCustomLayoutAlignHCenter(bool center)
{
    d_ptr->m_titleBarLayout->setCustomWidgetCenter(center);
}

QSize TitlebarWindow::sizeHint() const
{
    QSize sizeHint = QWidget::sizeHint();
    return sizeHint;
}

void TitlebarWindow::setTitleBarHeight(int height)
{
    d_ptr->m_titlebarWidget->setFixedHeight(height);
}

int TitlebarWindow::titleBarHeight()
{
    return d_ptr->m_titlebarWidget->height();
}

int TitlebarWindow::transparentWidth()
{
    return d_ptr->m_layout->margin();
}

}  // namespace KS
