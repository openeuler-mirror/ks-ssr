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
#ifndef TABLEDELETENOTIFY_H
#define TABLEDELETENOTIFY_H

#include <QWidget>
#include "src/ui/common/titlebar-window.h"

namespace Ui
{
class TableDeleteNotify;
}

namespace KS
{
class TableDeleteNotify : public TitlebarWindow
{
    Q_OBJECT

public:
    explicit TableDeleteNotify(QWidget *parent = nullptr);
    ~TableDeleteNotify();

private:
    void init();

signals:
    void accepted();

private:
    Ui::TableDeleteNotify *m_ui;
};
}  // namespace KS

#endif  // TABLEDELETENOTIFY_H