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

#include "src/ui/common/window/titlebar-window.h"

namespace Ui
{
class Login;
}

class QPushButton;
namespace KS
{
class LicenseProxy;
namespace Account
{
class Login : public TitlebarWindow
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    virtual ~Login();

    QString getPassword() const;
    void setPassword(const QString &password);
    QString getAccountName() const;
    void setAccountName(const QString &name);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void initUI();

private slots:
    void popupActiveDialog();

signals:
    void accepted();
    void rejected();

private:
    Ui::Login *m_ui;
    QPushButton *m_activateStatus;
    QSharedPointer<LicenseProxy> m_licenseProxy;
};
}  // namespace Account
}  // namespace KS
