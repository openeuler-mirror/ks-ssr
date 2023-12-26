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
struct PrivacyCleanupInfo
{
    // 是否选中
    bool selected;
    // 用户名
    QString userName;
    // 用户类型
    QString userType;
};

class PrivacyCleanupDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    PrivacyCleanupDelegate(QObject *parent = 0);
    virtual ~PrivacyCleanupDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

class PrivacyCleanupFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    PrivacyCleanupFilterModel(QObject *parent = nullptr);
    virtual ~PrivacyCleanupFilterModel(){};

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class PrivacyCleanupModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PrivacyCleanupModel(QObject *parent = nullptr);
    virtual ~PrivacyCleanupModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int getPrivacyCleanupInfosSize();
    QStringList getCheckedUserName();
    void setInfos(const QList<PrivacyCleanupInfo> &infos);
    void delcheckedInfos();

signals:
    void tableUpdated(int total);
    void stateChanged(Qt::CheckState checkState);

private:
    void checkSelectStatus();

private:
    QList<PrivacyCleanupInfo> m_infos;
};

class PrivacyCleanupTable : public QTableView
{
    Q_OBJECT

public:
    PrivacyCleanupTable(QWidget *parent = nullptr);
    virtual ~PrivacyCleanupTable(){};

    PrivacyCleanupFilterModel *getFilterProxy()
    {
        return m_filterProxy;
    };
    void setSearchText(const QString &text);
    int getPrivacyCleanupInfosSize();
    void cleanCheckedUsers();

signals:
    void tableUpdated(int total);

private:
    void initTable();
    QList<PrivacyCleanupInfo> getTableInfos();

private slots:
    void checkedAllItem(Qt::CheckState checkState);

private:
    PrivacyCleanupFilterModel *m_filterProxy;
    PrivacyCleanupModel *m_model;
    TableHeaderProxy *m_headerViewProxy;
    ToolBoxDbusProxy *m_dbusProxy;
};
}  // namespace ToolBox
}  // namespace KS
