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

#include <QWidget>

namespace Ui
{
class Home;
}

class BRDbusProxy;

namespace KS
{
namespace BR
{
class Home : public QWidget
{
    Q_OBJECT

public:
    Home(QWidget *parent = nullptr);
    virtual ~Home();

private:
    void init();

signals:
    void systemScanClicked();
    void customScanClicked();
    void currentStrategyChanged(int type);

public slots:
    void modfiyReinforcementTime();

private:
    Ui::Home *m_ui;
    BRDbusProxy *m_dbusProxy;
};
}  // namespace BR
}  // namespace KS
