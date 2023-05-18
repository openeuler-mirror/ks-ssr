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
#ifndef TRUSTEDSETTINGS_H
#define TRUSTEDSETTINGS_H

#include <QWidget>
#include <QButtonGroup>

namespace Ui
{
class TrustedSettings;
}

class KSSDbusProxy;

namespace KS
{
class TrustedUserPin;

class TrustedSettings : public QWidget
{
    Q_OBJECT

public:
    TrustedSettings(QWidget *parent = nullptr);
    ~TrustedSettings();

private:
    void init();
    void updateSoftMode();
    bool checkTrustedLoadFinied();

private slots:
    void switchChanged(bool checked);
    void softRadioChanged(bool checked);
    void hardRadioChanged(bool checked);
    void inputSoftUserPinAccepted();
    void inputHardUserPinAccepted();

private:
    Ui::TrustedSettings *m_ui;

    TrustedUserPin *m_userPin;
    KSSDbusProxy *m_kssDbusProxy;
};
}  // namespace KS
#endif  // TRUSTEDSETTINGS_H
