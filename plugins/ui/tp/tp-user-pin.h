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

#include <QCloseEvent>
#include <QWidget>
#include "include/ssr-i.h"
#include "lib/widgets/window/titlebar-window.h"

namespace Ui
{
class TPUserPin;
}

namespace KS
{
namespace TP
{
class TPUserPin : public TitlebarWindow
{
    Q_OBJECT

public:
    TPUserPin(QWidget *parent = nullptr);
    ~TPUserPin();

    QString getUserPin();
    SSRKSSTrustedStorageType getType();
    void setType(uint type);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void initUI();

signals:
    void accepted();
    void closed();

private:
    Ui::TPUserPin *m_ui;

    SSRKSSTrustedStorageType m_type;
};
}  // namespace TP
}  // namespace KS
