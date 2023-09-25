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

#include <QPushButton>
#include <QTableView>
#include <QWidget>

namespace Ui
{
class TableCommon;
}  // namespace Ui

namespace KS
{
class TableCommon : public QWidget
{
    Q_OBJECT
public:
    TableCommon(QWidget *parent = nullptr);
    virtual ~TableCommon(){};

public:
    void addOperationButton(QList<QPushButton *> btns);
    void addTable(QTableView *table);
    void setPrompt(const QString &prompt);
    void setSumTest(const QString &test);

signals:
    void sigSearchTextChanged(const QString &text);

private Q_SLOTS:

private:
    Ui::TableCommon *m_ui;
};
}  // namespace KS
