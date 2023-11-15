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

#include <QLabel>
#include <QWidget>
namespace KS
{
namespace BR
{
namespace Reports
{
class RoundProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit RoundProgressBar(const QString &name,
                              int total,
                              int conform,
                              int inconform,
                              QWidget *parent = nullptr);
    virtual QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *);

private:
    void initUI();

private:
    // 比率label
    QLabel *m_percentLabel;
    // 分类名label
    QLabel *m_nameLabel;
    // 详情label 符合与不符合项
    QLabel *m_noteLabel;
    // 比率
    float m_percent = 0.00;
    QString m_name;
    int m_total;
    int m_conform;
    int m_inconform;
};
}  // namespace Reports
}  // namespace BR
}  // namespace KS
