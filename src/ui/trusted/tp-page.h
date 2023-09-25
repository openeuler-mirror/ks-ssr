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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#pragma once

#include <QWidget>
#include <QListWidgetItem>

namespace Ui
{
class TPPage;
}

namespace KS
{
class SidebarItem;
class TPKernel;
class TPExecute;

class TPPage : public QWidget
{
    Q_OBJECT

public:
    TPPage(QWidget *parent = nullptr);
    virtual ~TPPage();

protected:
    void paintEvent(QPaintEvent *event);

private:
    void initSidebar();
    void initSubPage();
    void checkTrustedLoadFinied();
    void createSideBarItem(const QString &text, const QString &icon);

private slots:
    void onItemClicked(QListWidgetItem *currItem);

private:
    Ui::TPPage *m_ui;
    QMap<QString, SidebarItem *> m_sidebarItems;
};
}  // namespace KS
