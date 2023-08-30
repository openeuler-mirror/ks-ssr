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

#include "src/ui/common/titlebar-window.h"

namespace Ui
{
class RetrieveBoxPassword;
}

namespace KS
{
class RetrieveBoxPassword : public TitlebarWindow
{
    Q_OBJECT

public:
    explicit RetrieveBoxPassword(QWidget *parent = nullptr);
    ~RetrieveBoxPassword();
    QString getPassphrase();

private:
    void init();

private slots:
    void onOkClicked();

signals:
    void accepted();
    void rejected();
    // 输入空字符
    void inputEmpty();

private:
    Ui::RetrieveBoxPassword *m_ui;
};
}  // namespace KS
