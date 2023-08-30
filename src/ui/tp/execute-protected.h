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

#include <QTimer>
#include <QWidget>
#include "src/ui/tp/execute-protected-table.h"

namespace Ui
{
class ExecuteProtected;
}

class TPProxy;

namespace KS
{
class ExecuteProtected : public QWidget
{
    Q_OBJECT
public:
    ExecuteProtected(QWidget *parent = nullptr);
    ~ExecuteProtected();

    bool getInitialized();

private:
    void updateTips(int total);
    bool isExistSelectedItem();

signals:
    void initFinished();
private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void addExecuteFile(bool checked);
    void updateExecuteList(bool checked);
    void recertification(bool checked);
    void popDeleteNotify(bool checked);
    void removeExecuteFiles();
    void updateRefreshIcon();

private:
    Ui::ExecuteProtected *m_ui;

    KSSDbusProxy *m_dbusProxy;
    QTimer *m_refreshTimer;
};
}  // namespace KS
