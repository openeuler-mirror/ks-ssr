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
class ProgressIcon : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressIcon(QWidget *parent = nullptr);
    ~ProgressIcon();

    void finishedProgress(bool isFinish);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();

private slots:
    void circlePixmap();

private:
    QTimer *m_timer;
    QLabel *m_circlePixmapLabel;
    QPixmap m_circlePixmap;
    bool m_isFinishedProgress;
};
}  // namespace BR
}  // namespace KS
