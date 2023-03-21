/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiranwidgets-qt5 is licensed under Mulan PSL v2.
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

#pragma once

#include <QLabel>
#include <QLayout>
#include <QObject>
#include <QPushButton>

#include "global-define.h"
#include "title-bar-layout.h"
#include "titlebar-window.h"

class QGraphicsDropShadowEffect;

namespace KS
{
class TitlebarWindowPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(TitlebarWindow);

public:
    TitlebarWindowPrivate(TitlebarWindow *ptr);
    ~TitlebarWindowPrivate();

private:
    void init();

    void setIcon(const QIcon &icon);
    void setTitle(const QString &title);
    void setButtonHints(TitlebarWindow::TitlebarButtonHintFlags hints);
    void setWindowContentWidget(QWidget *widget);

private:
    void handlerHoverMoveEvent(QHoverEvent *ev);
    void handlerLeaveEvent();
    void handlerMouseButtonPressEvent(QMouseEvent *ev);
    void handlerMouseButtonReleaseEvent(QMouseEvent *ev);
    void handlerMouseMoveEvent(QMouseEvent *ev);
    void handlerMouseDoubleClickEvent(QMouseEvent *ev);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void initOtherWidget();
    void initShadow();
    void updateShadowStyle(bool active);
    KS::CursorPositionEnums getCursorPosition(QPoint pos);

private slots:
    void updateTitleFont(QFont font);

private:
    TitlebarWindow *q_ptr;
    QLayout *m_layout; /** < 主布局 **/

    QFrame *m_frame;        /** < 主背景 **/
    QLayout *m_frameLayout; /** < 主背景布局 **/

    QWidget *m_titlebarWidget; /** < 标题栏窗口 **/
    TitlebarLayout *m_titleBarLayout;

    QLabel *m_titleIcon; /** < 标题栏图标 **/
    QLabel *m_title;     /** < 标题栏文本 **/

    QWidget *m_titlebarCenterWidget; /** < 标题栏中间组件 **/
    QHBoxLayout *m_customLayout;     /** < 标题栏中间组件给使用者的可自定义的控件 **/

    QWidget *m_titlebarRirghtWidget;                       /** < 标题栏右侧组件 **/
    TitlebarWindow::TitlebarButtonHintFlags m_buttonHints; /** < 右侧显示按钮枚举 **/
    QPushButton *m_btnMin;
    QPushButton *m_btnMax;
    QPushButton *m_btnClose;

    QWidget *m_windowContentWidgetWrapper;
    QWidget *m_windowContentWidget;

private:
    bool m_titlebarIsPressed;
    bool m_resizeable;
    // NOTE: 使用DropShadowEffect会将设置的窗口绘制成图片，之后再背景透明的地方添加阴影,再进行渲染
    // 理论上窗口内容m_frame绘制的圆角窗口具有背景，所以不会在m_frame子控件里添加阴影
    QGraphicsDropShadowEffect *m_shadowEffect;
};

}  // namespace KS