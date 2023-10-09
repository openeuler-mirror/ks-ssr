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
class BoxPasswordChecked;
}
namespace KS
{
namespace BOX
{
class BoxPasswordChecked : public TitlebarWindow
{
    Q_OBJECT

public:
    explicit BoxPasswordChecked(QWidget *parent = nullptr);
    ~BoxPasswordChecked();

    QString getBoxPasswordChecked();

private:
    void init();

signals:
    void accepted();
    void rejected();

private:
    Ui::BoxPasswordChecked *m_ui;
};
}  // namespace BOX
}  // namespace KS
