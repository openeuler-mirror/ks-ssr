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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */
#ifndef INPUTPASSWORD_H
#define INPUTPASSWORD_H

#include <QWidget>

namespace Ui
{
class InputPassword;
}
namespace KS
{
class InputPassword : public QWidget
{
    Q_OBJECT

public:
    explicit InputPassword(QWidget *parent = nullptr);
    ~InputPassword();

    QString getInputPassword();

private:
    void init();

signals:
    void accepted();
    void rejected();

private:
    Ui::InputPassword *m_ui;
};
}  // namespace KS

#endif  // INPUTPASSWORD_H
