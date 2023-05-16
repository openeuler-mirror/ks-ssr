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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QWidget>

class KSSDbusProxy;

namespace Ui
{
class FPPage;
}  // namespace Ui

namespace KS
{
class FPPage : public QWidget
{
    Q_OBJECT
public:
    FPPage(QWidget *parent = nullptr);
    virtual ~FPPage();

private:
    void updateInfo();
    bool checkTrustedLoadFinied(bool initialized);

private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void addClicked(bool checked);
    void unprotectClicked(bool checked);
    void unprotectAccepted();

private:
    Ui::FPPage *m_ui;

    KSSDbusProxy *m_fileProtectedProxy;
};
}  // namespace KS
