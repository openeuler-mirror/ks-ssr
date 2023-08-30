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

#include <QHeaderView>
namespace KS
{
class TableHeaderProxy : public QHeaderView
{
    Q_OBJECT
public:
    explicit TableHeaderProxy(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void paintSection(QPainter *painter,
                      const QRect &rect,
                      int logicalIndex) const override;

signals:
    void toggled(Qt::CheckState checkState);

public slots:
    void setCheckState(Qt::CheckState checkState);

private:
    bool m_stateChanged;
    Qt::CheckState m_checkState;

    QRect *m_rect;
};

}  // namespace KS
