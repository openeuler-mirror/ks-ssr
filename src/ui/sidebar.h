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

#include <QListWidget>
#include <QWidget>

namespace Ui
{
class SidebarItem;
}
namespace KS
{
class SidebarItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectChanged)
public:
    struct ItemInfo
    {
        QString name;
        QString icon;
    };

    SidebarItem(const ItemInfo &itemInfo,
                QWidget *parent = nullptr);
    virtual ~SidebarItem();

public:
    void setSelected(bool isSelected);
    bool isSelected();
    QString getItemUID();

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::SidebarItem *m_ui;
    bool m_isSelected;

signals:
    void selectChanged(bool isSelected);
};

class SideBar : public QListWidget
{
    Q_OBJECT

public:
    SideBar(QWidget *parent = nullptr);
    virtual ~SideBar(){};

    // 侧边栏添加按钮
    void addSideBarItem(SidebarItem *item);
    QString getSelectedUID();

private slots:
    void setSideBarItemStatus(int currentRow);

signals:
    void itemChanged();
};

}  // namespace KS
