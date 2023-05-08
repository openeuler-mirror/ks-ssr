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

#ifndef KIRANSTYLE_SWITCH_BUTTON_H
#define KIRANSTYLE_SWITCH_BUTTON_H

#include <QAbstractButton>

class QStyleOptionButton;

namespace KS
{
class KiranSwitchButtonPrivate;

/**
 * @brief SwitchButton开关按钮，仅在KiranStyle下生效，其他Style中仅绘制成Button
 * <img src="../snapshot/kiran-switch-button.png" alt="kiran-switch-button" style="zoom:90%;" />
 *
 * 使用说明
 *
 * 通过 `isChecked` 方法获取开关状态,通过 `setChecked`　方法来设置开关状态
 * 通过连接到QAbstractButton::toggled(bool checked)来处理开关变化
 *
 * @since 2.1.0
 * @see QAbstractButton
 */
class Q_DECL_EXPORT KiranSwitchButton : public QAbstractButton
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KiranSwitchButton);

public:
    /**
     * @brief KiranSwitchButton构造方法
     * @param parent 父控件
     */
    explicit KiranSwitchButton(QWidget *parent = nullptr);
    /**
     * @brief KiranSwitchButton析构方法
     */
    ~KiranSwitchButton();

    /**
     * @see QWidget::sizeHint()
     */
    virtual QSize sizeHint() const override;

    /**
     * @see QWidget::minimumSizeHint()
     */
    virtual QSize minimumSizeHint() const override;

private:
    void initStyleOption(QStyleOptionButton *option) const;
    bool event(QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    /* KiranSwitchButton私有类指针 */
    KiranSwitchButtonPrivate *d_ptr;
};
}  // namespace KS

#endif  //KIRANSTYLE_SWITCH_BUTTON_H
