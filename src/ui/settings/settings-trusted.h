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

#include <QButtonGroup>
#include <QWidget>

namespace Ui
{
class SettingsTrusted;
}

class KSSDbusProxy;

namespace KS
{
class TrustedUserPin;

class SettingsTrusted : public QWidget
{
    Q_OBJECT

public:
    SettingsTrusted(QWidget *parent = nullptr);
    ~SettingsTrusted();

private:
    void initUI();
    void updateStorageMode();
    bool checkTrustedLoadFinied();

private slots:
    void setTrustedStatus(bool checked);
    void updateSoftRadio(bool checked);
    void updateHardRadio(bool checked);
    void setStorageMode();

private:
    Ui::SettingsTrusted *m_ui;

    TrustedUserPin *m_userPin;
    KSSDbusProxy *m_kssDbusProxy;
};
}  // namespace KS