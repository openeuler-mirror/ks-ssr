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
struct FileShredInfo
{
    // 是否选中
    bool selected;
    // 用户名
    QString fileName;
    // 日期时间
    QString filePath;
};

class FileShredDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    FileShredDelegate(QObject *parent = 0);
    virtual ~FileShredDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

class FileShredFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FileShredFilterModel(QObject *parent = nullptr);
    virtual ~FileShredFilterModel(){};

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class FileShredModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    FileShredModel(QObject *parent = nullptr);
    virtual ~FileShredModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int getFileShredInfosSize();
    QStringList getCheckedPath();
    void addFiles(const QStringList &paths);
    void delFiles();

signals:
    void tableUpdated(int total);
    void stateChanged(Qt::CheckState checkState);

private:
    void checkSelectStatus();

private:
    QList<FileShredInfo> m_infos;
};

class FileShredTable : public QTableView
{
    Q_OBJECT

public:
    FileShredTable(QWidget *parent = nullptr);
    virtual ~FileShredTable(){};

    FileShredFilterModel *getFilterProxy()
    {
        return m_filterProxy;
    };
    void setSearchText(const QString &text);
    int getFileShredInfosSize();
    void addFiles(const QStringList &paths);
    void delFiles();
    void shredFiles();

signals:
    void tableUpdated(int total);

private:
    void initTable();

private slots:
    void checkedAllItem(Qt::CheckState checkState);

private:
    FileShredFilterModel *m_filterProxy;
    FileShredModel *m_model;
    TableHeaderProxy *m_headerViewProxy;
    ToolBoxDbusProxy *m_dbusProxy;
};
}  // namespace ToolBox
}  // namespace KS
