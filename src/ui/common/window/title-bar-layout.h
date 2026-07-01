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
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#pragma once

#include <QLabel>
#include <QLayout>
#include <QMargins>

namespace KS
{
class TitlebarLayout : public QLayout
{
    Q_OBJECT
public:
    TitlebarLayout(QWidget* parent = nullptr);
    ~TitlebarLayout();

    QLayoutItem* titleBarIcon();
    void setTitleBarIconLabel(QLabel* icon);
    // NOTE:上下边距不会生效,生效的只有左右边距
    QMargins titleBarIconMargin();
    void setTitleBarIconMargin(QMargins margins);

    QLayoutItem* titleBarTitle();
    void setTitleBarTitleLabel(QLabel* title);
    void setTitleBarCompleteTitle(const QString& title);
    QMargins titleBarLabelMargin();
    void setTitleBarTLabelMargin(QMargins margins);

    QLayoutItem* titleBarCustomWidget();
    void setTitleBarCustomWidget(QWidget* customWidget);
    QMargins titleBarCustomMargin();
    void setTitleBarCustomMargin(QMargins margins);

    QLayoutItem* titleBarRightWidget();
    void setTitleBarRightWidget(QWidget* rightWidget);
    QMargins titleBarRightMargin();
    void setTitleBarRightWidgetMargin(QMargins margins);

    void setCustomWidgetCenter(bool center);
    bool customWidgetCenter();

protected:
    /**
     * @brief 将布局项添加到布局中
     * @param item
     */
    virtual void addItem(QLayoutItem* item) override;
    /**
     * @brief  控件的建议大小
     * @return 控件大小
     */
    virtual QSize sizeHint() const override;
    /**
     * @brief 获取索引处的布局项，如没有这样的项返回null
     * @param index
     * @return
     */
    virtual QLayoutItem* itemAt(int index) const override;
    /**
     * @brief 计算各个item的位置大小
     * @param rect
     */
    virtual void setGeometry(const QRect& rect) override;
    virtual QSize minimumSize() const override;
    virtual QLayoutItem* takeAt(int index) override;
    virtual int count() const override;

private:
    QMargins m_iconMargins;
    QLayoutItem* m_iconLabelItem = nullptr;

    QMargins m_titleMargins;
    QString m_completeTitle;
    QLayoutItem* m_titleLabelItem = nullptr;

    QMargins m_customWidgetMargins;
    bool m_customWidgetCenter = true;
    QLayoutItem* m_customWidgetItem = nullptr;

    QMargins m_rightWidgetMargins;
    QLayoutItem* m_rightWidgetItem = nullptr;
};

}  // namespace KS