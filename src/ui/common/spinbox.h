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

#include <QObject>
#include <QSpinBox>

namespace KS
{
// TODO：使用eventfilter实现，无需再封装一个类
// 无实际意义 用于屏蔽鼠标滚动
class SpinBox : public QSpinBox
{
    Q_OBJECT

public:
    SpinBox(QWidget *parent = nullptr);
    virtual ~SpinBox();

protected:
    void wheelEvent(QWheelEvent *event) override;
};
}  // namespace KS
