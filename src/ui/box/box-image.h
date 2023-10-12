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

#include <QWidget>

namespace KS
{
namespace Box
{
class BoxImage : public QWidget
{
    Q_OBJECT
public:
    explicit BoxImage(QWidget *parent = nullptr, const QString &imagePath = "");

protected:
    void paintEvent(QPaintEvent *event);

private:
    QImage *m_image;
    QRect m_drawArea;  // 绘制区域
};
}  // namespace Box
}  // namespace KS