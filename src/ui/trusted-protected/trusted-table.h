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
#ifndef TPTABLE_H
#define TPTABLE_H

#include <QAbstractTableModel>
#include <QList>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QWidget>
#include "src/ui/trusted-protected/new-headerview.h"
#include "src/ui/trusted-protected/table-common.h"

class TrustedProxy;

namespace KS
{
enum TrustedProtectType
{
    TRUSTED_PROTECT_EXECUTE = 0,
    TRUSTED_PROTECT_KERNEL,
    TRUSTED_PROTECT_NONE
};

enum TrustedFileType
{
    // 未知文件类型
    TRUSTED_FILE_TYPE_NONE = 0,
    // 可执行文件
    TRUSTED_FILE_TYPE_EXECUTABLE_FILE,
    // 动态库
    TRUSTED_FILE_TYPE_DYNAMIC_LIBRARY,
    // 内核模块
    TRUSTED_FILE_TYPE_KERNEL_MODULE,
    // 可执行脚本
    TRUSTED_FILE_TYPE_EXECUTABLE_SCRIPT
};

enum TrustedFileStatus
{
    // 异常 (未认证)
    ILLEGAL_STATUS = 0,
    // 正常（已认证）
    NORMAL_STATUS,
};

struct TrustedInfo
{
    // 是否被选中
    bool selected;
    // 文件路径
    QString filePath;
    // 文件类型
    QString type;
    // 状态
    QString status;
    // 是否开启防卸载
    // bool
};

class TPDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    TPDelegate(QObject *parent = 0, TrustedFileStatus status = NORMAL_STATUS);
    virtual ~TPDelegate();

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
};

class TPFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    TPFilterModel(QObject *parent = nullptr, TrustedProtectType type = TRUSTED_PROTECT_KERNEL);
    virtual ~TPFilterModel(){};

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    TrustedProtectType m_type;
};

class TPModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TPModel(QObject *parent = nullptr, TrustedProtectType type = TRUSTED_PROTECT_KERNEL);
    virtual ~TPModel(){};

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

    void updateInfo();

    QList<TrustedInfo> getTrustedInfos();

signals:
    void stateChanged(Qt::CheckState checkState);

public slots:
    void onStateChanged(Qt::CheckState checkState);

private:
    void onSingleStateChanged();

private:
    TrustedProxy *m_trustedProtectedProxy;
    QList<TrustedInfo> m_trustedInfos;
    TrustedProtectType m_type;
};

class TPTable : public QTableView
{
    Q_OBJECT

public:
    TPTable(QWidget *parent = nullptr, TrustedProtectType type = TRUSTED_PROTECT_KERNEL);
    virtual ~TPTable(){};

    void searchTextChanged(const QString &text);
    void updateInfo();
    QList<TrustedInfo> getTrustedInfos();

private:
    void mouseEnter(const QModelIndex &index);

private:
    TPFilterModel *m_filterProxy;
    TPModel *m_model;
    NewHeaderView *m_newHeaderView;
};

}  // namespace KS

#endif  // TPTABLE_H
