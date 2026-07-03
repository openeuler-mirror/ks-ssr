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

#ifndef KIRANSTYLE_SWITCH_BUTTON_PRIVATE_H
#define KIRANSTYLE_SWITCH_BUTTON_PRIVATE_H

#include "kiran-switch-button.h"

namespace KS
{
class KiranSwitchButtonPrivate
{
    Q_DECLARE_PUBLIC(KiranSwitchButton)
public:
    KiranSwitchButtonPrivate(KiranSwitchButton* ptr)
        : q_ptr(ptr)
    {
    }
    ~KiranSwitchButtonPrivate() = default;

    /**
     * 计算布局，返回尺寸大小
     * \param indicatorRect             按钮指示器矩形
     * \param indicatorCircularRect     按钮指示器之中的圆形区域矩形
     * \param textRect                  文本矩形
     * \return 总计大小
     */
    void doLayout(QRect& indicatorRect, QRect& indicatorCircularRect, QRect& textRect);

private:
    KiranSwitchButton* q_ptr;
};
}  // namespace KS
#endif  //KIRANSTYLE_SWITCH_BUTTON_PRIVATE_H
