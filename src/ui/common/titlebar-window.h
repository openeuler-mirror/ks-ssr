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
#pragma once

#include <QWidget>

class QHBoxLayout;

namespace KS
{
class TitlebarWindowPrivate;

class Q_DECL_EXPORT TitlebarWindow : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TitlebarWindow)
    Q_DISABLE_COPY(TitlebarWindow)
public:
    /**
     * @brief 标题栏按钮选项
     */
    enum TitlebarButtonHint
    {
        TitlebarMinimizeButtonHint = 0x00000001,                                                                     /** < 最小化 */
        TitlebarMaximizeButtonHint = 0x00000002,                                                                     /** < 最大化 */
        TitlebarCloseButtonHint = 0x00000004,                                                                        /** < 关闭 */
        TitlebarMinMaxCloseHints = TitlebarMinimizeButtonHint | TitlebarMaximizeButtonHint | TitlebarCloseButtonHint /** < 最小化,最大化,关闭 */
    };
    Q_ENUM(TitlebarButtonHint)
    Q_DECLARE_FLAGS(TitlebarButtonHintFlags, TitlebarButtonHint)
    Q_FLAG(TitlebarButtonHintFlags)

public:
    /**
     * @brief 构造方法
     * @param parent      父控件
     * @param windowFlags window标志
     * @see QWidget::QWidget(QWidget*, Qt::WindowFlags)
     */
    explicit TitlebarWindow(QWidget *parent = nullptr, Qt::WindowFlags windowFlags = Qt::Window);

    /**
     * @brief 析构方法
     */
    virtual ~TitlebarWindow();

    /**
     * @brief 设置自定义标题栏的内容窗口
     * @param widget 需要设置的内容窗口
     */
    void setWindowContentWidget(QWidget *widget);

    /**
     * @brief  获取窗口内容部件
     * @return 窗口内容部件
     */
    QWidget *getWindowContentWidget();

    /**
     * @brief  获取标题栏部分自定义布局，可自行添加控件至标题栏
     * @return 窗口内容部件布局
     */
    QHBoxLayout *getTitlebarCustomLayout();

    /**
     * @brief 获取标题栏部分自定义控件区域是否居中
     * @return 标题栏自定义控件区域是否居中
     */
    bool titlebarCustomLayoutAlignHCenter();

    /**
     * @brief 设置标题栏自定义控件区域是否居中
     * @param center 自定义控件区域是否居中
     */
    void setTitlebarCustomLayoutAlignHCenter(bool center);

    /**
     * @brief 设置窗口图标
     * @param icon 窗口图标
     */
    void setIcon(const QIcon &icon);

    /**
     * @brief 设置标题栏
     * @param text 标题栏文本
     */
    void setTitle(const QString &text);

    /**
     * @brief 设置标题栏按钮
     * @param hints 标题栏按钮flag
     */
    void setButtonHints(TitlebarButtonHintFlags hints);

    /**
     * @brief 设置窗口是否允许改变大小
     * @param resizeable 是否可改变大小
     */
    void setResizeable(bool resizeable);

    QSize sizeHint() const override;

    void setTitleBarHeight(int height);

    int titleBarHeight();

    /**
     * 获取窗口透明边框的宽度
     */
    int transparentWidth();

protected:
    /* 事件处理 */
    bool event(QEvent *event) override;

private:
    /* TitlebarWindow私有类指针 */
    TitlebarWindowPrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TitlebarWindow::TitlebarButtonHintFlags)

}  // namespace KS