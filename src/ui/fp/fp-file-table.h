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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QAbstractTableModel>
#include <QList>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QWidget>
#include "src/ui/tp/tp-table-header-proxy.h"

class FileProtectedProxy;

namespace KS
{
struct FPFileInfo
{
    // 是否被选中
    bool selected;
    // 文件名
    QString fileName;
    // 文件路径
    QString filePath;
    // 添加事件
    QString addTime;
};

class FPFilesDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    FPFilesDelegate(QObject *parent = 0);
    virtual ~FPFilesDelegate();

    // QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    // void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    // void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    // void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class FPFilesFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FPFilesFilterModel(QObject *parent = nullptr);
    virtual ~FPFilesFilterModel(){};

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class FPFilesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    FPFilesModel(QObject *parent = nullptr);
    virtual ~FPFilesModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    //    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void updateInfo();

    QList<FPFileInfo> getFPFileInfos();

signals:
    void stateChanged(Qt::CheckState checkState);

public slots:
    void onStateChanged(Qt::CheckState checkState);

private:
    void onSingleStateChanged();

private:
    FileProtectedProxy *m_fileProtectedProxy;
    QList<FPFileInfo> m_filesInfo;
};

class FPFileTable : public QTableView
{
    Q_OBJECT

public:
    FPFileTable(QWidget *parent = nullptr);
    virtual ~FPFileTable(){};

    FPFilesFilterModel *getFilterProxy() { return m_filterProxy; };
    void searchTextChanged(const QString &text);
    void updateInfo();
    QList<FPFileInfo> getFPFileInfos();

private:
    void mouseEnter(const QModelIndex &index);

private:
    FPFilesFilterModel *m_filterProxy;
    FPFilesModel *m_model;
    TPTableHeaderProxy *m_headerViewProxy;
};

}  // namespace KS
