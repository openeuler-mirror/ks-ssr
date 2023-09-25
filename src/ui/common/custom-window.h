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
#ifndef CUSTOMWINDOW_H
#define CUSTOMWINDOW_H

#include <QVBoxLayout>
#include "src/ui/common/titlebar-window.h"

namespace KS
{
class CustomWindow : public TitlebarWindow
{
    Q_OBJECT
public:
    CustomWindow(QWidget *parent = nullptr);
    virtual ~CustomWindow();

    QVBoxLayout *getContentLayout();

    void buildNotify(const QString &notify);
private:
    void initUI();

private:
    QVBoxLayout *m_contentLayout;
};
}  // namespace KS

#endif  // CUSTOMWINDOW_H
