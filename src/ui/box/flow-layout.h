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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QLayout>
#include <QMap>
#include <QPair>
#include <QStyle>

namespace KS
{
class FlowLayout : public QLayout
{
    Q_OBJECT

public:
    explicit FlowLayout(QWidget *parent = nullptr, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    virtual ~FlowLayout();

public:
    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

private:
    int doLayout(const QRect &rect) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

private:
    QList<QLayoutItem *> m_itemList;
    int m_hSpace;
    int m_vSpace;
};

}  // namespace KS
