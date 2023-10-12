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
#include <QTableView>
#include <QWidget>
#include "src/ui/common/table-header-proxy.h"
#include "src/ui/tp/utils.h"

class KSSDbusProxy;

namespace KS
{
namespace TP
{
class KernelProtectedFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    KernelProtectedFilterModel(QObject *parent = nullptr);
    virtual ~KernelProtectedFilterModel(){};

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class KernelProtectedModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    KernelProtectedModel(QObject *parent = nullptr);
    virtual ~KernelProtectedModel(){};

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

    QList<TrustedRecord> getKernelRecords();

signals:
    void stateChanged(Qt::CheckState checkState);
    void filesUpdate(int total);

private:
    void checkSelectStatus();

private:
    KSSDbusProxy *m_tpDBusProxy;
    QList<TrustedRecord> m_kernelRecords;
};

class KernelProtectedTable : public QTableView
{
    Q_OBJECT

public:
    KernelProtectedTable(QWidget *parent = nullptr);
    virtual ~KernelProtectedTable(){};

    void searchTextChanged(const QString &text);
    void updateInfo();
    QList<TrustedRecord> getKernelRecords();
    // 获取被篡改数
    int getKerneltamperedNums();

signals:
    void prohibitUnloadingStatusChange(bool status, const QString &path);
    void filesUpdate(int total);

private slots:
    void itemEntered(const QModelIndex &index);
    void itemClicked(const QModelIndex &index);

private slots:
    void checkedAllItem(Qt::CheckState checkState);

private:
    KernelProtectedFilterModel *m_filterProxy;
    KernelProtectedModel *m_model;
    TableHeaderProxy *m_headerViewProxy;
};

}  // namespace TP
}  // namespace KS
