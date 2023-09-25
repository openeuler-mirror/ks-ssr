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

#ifndef TPKERNELTABLE_H
#define TPKERNELTABLE_H

#include <QAbstractTableModel>
#include <QList>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QWidget>
#include "src/ui/trusted/tp-table-header-proxy.h"
#include "src/ui/trusted/tp-utils.h"

class TrustedProxy;

namespace KS
{
class TPKernelFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    TPKernelFilterModel(QObject *parent = nullptr);
    virtual ~TPKernelFilterModel(){};

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class TPKernelModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TPKernelModel(QObject *parent = nullptr);
    virtual ~TPKernelModel(){};

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

public slots:
    void onStateChanged(Qt::CheckState checkState);

private:
    void onSingleStateChanged();

private:
    TrustedProxy *m_trustedProtectedProxy;
    QList<TrustedRecord> m_kernelRecords;
};

class TPKernelTable : public QTableView
{
    Q_OBJECT

public:
    TPKernelTable(QWidget *parent = nullptr);
    virtual ~TPKernelTable(){};

    void searchTextChanged(const QString &text);
    void updateRecord();
    QList<TrustedRecord> getKernelRecords();

private:
    void mouseEnter(const QModelIndex &index);

private:
    TPKernelFilterModel *m_filterProxy;
    TPKernelModel *m_model;
    TPTableHeaderProxy *m_headerViewProxy;
};

}  // namespace KS

#endif  // TPKERNELTABLE_H
