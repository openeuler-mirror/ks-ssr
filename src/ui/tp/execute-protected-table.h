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
#include <QWidget>
#include "src/ui/tp/table-header-proxy.h"
#include "src/ui/tp/tp-utils.h"

class KSSDbusProxy;

namespace KS
{
class ExecuteProtectedFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ExecuteProtectedFilterModel(QObject *parent = nullptr);
    virtual ~ExecuteProtectedFilterModel(){};

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class ExecuteProtectedModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ExecuteProtectedModel(QObject *parent = nullptr);
    virtual ~ExecuteProtectedModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;
    //    bool setHeaderData(int section,
    //                       Qt::Orientation orientation,
    //                       const QVariant &value,
    //                       int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void updateRecord();

    QList<TrustedRecord> getExecuteRecords();

signals:
    void stateChanged(Qt::CheckState checkState);
    void filesUpdate(int total);

private:
    void checkSelectStatus();

private:
    KSSDbusProxy *m_tpDBusProxy;
    QList<TrustedRecord> m_executeRecords;
};

class ExecuteProtectedTable : public QTableView
{
    Q_OBJECT

public:
    ExecuteProtectedTable(QWidget *parent = nullptr);
    virtual ~ExecuteProtectedTable(){};

    void searchTextChanged(const QString &text);
    void updateInfo();
    QList<TrustedRecord> getExecuteRecords();
    int getExecutetamperedNums();

private:
    void showDetails(const QModelIndex &index);

signals:
    void filesUpdate(int total);

private slots:
    void checkedAllItem(Qt::CheckState checkState);

private:
    ExecuteProtectedFilterModel *m_filterProxy;
    ExecuteProtectedModel *m_model;
    TableHeaderProxy *m_headerViewProxy;
};

}  // namespace KS
