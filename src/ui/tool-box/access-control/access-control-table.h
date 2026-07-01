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

#include <QAbstractTableModel>
#include <QList>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTableView>

class ToolBoxDbusProxy;

namespace KS
{
class TableHeaderProxy;

namespace ToolBox
{
struct AccessControlInfo
{
    // 用户名
    QString userName;
    // 用户功能
    QString userFunction;
};

class AccessControlDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    AccessControlDelegate(QObject *parent = 0);
    virtual ~AccessControlDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class AccessControlModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    AccessControlModel(QObject *parent = nullptr);
    virtual ~AccessControlModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void showInfos(bool isShow);

private:
    QList<AccessControlInfo> m_infos;
};

class AccessControlTable : public QTableView
{
    Q_OBJECT

public:
    AccessControlTable(QWidget *parent = nullptr);
    virtual ~AccessControlTable(){};

    bool openSelinux(bool isOpen);
    bool getSelinuxStatus();

    void showTable(bool isShow);

private:
    void initTable();

private:
    AccessControlModel *m_model;
    TableHeaderProxy *m_headerViewProxy;
    ToolBoxDbusProxy *m_dbusProxy;
};
}  // namespace ToolBox
}  // namespace KS
