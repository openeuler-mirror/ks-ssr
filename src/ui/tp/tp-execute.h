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

#include <QWidget>
#include "src/ui/tp/tp-execute-table.h"

namespace Ui
{
class TPExecute;
}

class TPProxy;

namespace KS
{
class TPExecute : public QWidget
{
    Q_OBJECT
public:
    TPExecute(QWidget *parent = nullptr);
    ~TPExecute();

    int getInitialized();

private:
    void updateInfo();
signals:
    void initFinished();
private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void addClicked(bool checked);
    void updateClicked(bool checked);
    void unprotectClicked(bool checked);
    void unprotectAccepted();

private:
    Ui::TPExecute *m_ui;

    KSSDbusProxy *m_dbusProxy;
};
}  // namespace KS
