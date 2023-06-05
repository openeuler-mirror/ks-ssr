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

#pragma once

#include <QCloseEvent>
#include <QWidget>
#include "include/ksc-i.h"
#include "src/ui/common/titlebar-window.h"

namespace Ui
{
class TrustedUserPin;
}

namespace KS
{
class TrustedUserPin : public TitlebarWindow
{
    Q_OBJECT

public:
    TrustedUserPin(QWidget *parent = nullptr);
    ~TrustedUserPin();

    QString getUserPin();
    KSCKSSTrustedStorageType getType();
    void setType(uint type);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void initUI();

signals:
    void accepted();
    void closed();

private:
    Ui::TrustedUserPin *m_ui;

    KSCKSSTrustedStorageType m_type;
};
}  // namespace KS
