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
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#pragma once

#include <page.h>
#include <QLabel>
#include <QMap>
#include <QPushButton>

class QPushButton;
class QLabel;
class QButtonGroup;

namespace KS
{
struct NavigationItemInfo
{
    NavigationIndex index;
    QString iconName;
    QString description;
};

class NavigationItem : public QWidget
{
    Q_OBJECT
public:
    NavigationItem(NavigationIndex index, const QString &iconName, const QString &description);
    virtual ~NavigationItem(){};

    NavigationIndex getIndex()
    {
        return m_index;
    }

    QAbstractButton *getButton()
    {
        return m_icon;
    };

    QString getDescription()
    {
        return m_description->text();
    };

Q_SIGNALS:
    void clicked(bool checked);

private:
    NavigationIndex m_index;
    QPushButton *m_icon;
    QLabel *m_description;
};

class Navigation : public QWidget
{
    Q_OBJECT

public:
    Navigation(QWidget *parent = nullptr);
    virtual ~Navigation(){};

    // 添加需要显示的分类项
    void setItems(QVector<NavigationIndex> indexs);

    NavigationIndex getSelectedIndex();
    void setBtnChecked(int id);

    void clearItems();

Q_SIGNALS:
    void currentUIDChanged();

private:
    void buildItems();

private:
    // 导航图标按钮组
    QButtonGroup *m_items;
    QMap<int, NavigationIndex> m_itemUIDs;
    QVector<NavigationItemInfo> m_navigationItemsInfo;
};

}  // namespace KS
