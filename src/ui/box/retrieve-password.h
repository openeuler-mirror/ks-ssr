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
#ifndef RETRIEVEPASSWORD_H
#define RETRIEVEPASSWORD_H

#include <QWidget>

namespace Ui
{
class RetrievePassword;
}

namespace KS
{
class RetrievePassword : public QWidget
{
    Q_OBJECT

public:
    explicit RetrievePassword(QWidget *parent = nullptr);
    ~RetrievePassword();
    QString getNewPassword();
    QString getPassphrase();

private:
    void init();

private slots:
    void onOkClicked();

signals:
    void accepted();
    void rejected();
    // 两次密码不一致
    void passwdInconsistent();
    // 输入空字符
    void inputEmpty();

private:
    Ui::RetrievePassword *m_ui;
};
}  // namespace KS

#endif  // RETRIEVEPASSWORD_H
