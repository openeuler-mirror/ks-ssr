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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#pragma once

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTableView>

class QWidget;
template <typename T>
class QList;

namespace KS
{
class TableHeaderProxy;

namespace ToolBox
{
enum FileSignField
{
    FILE_SIGN_FIELD_CHECKBOX = 0,
    FILE_SIGN_FIELD_FILE_PATH,
    FILE_SIGN_FIELD_FILE_SE_CONTEXT,
    FILE_SIGN_FIELD_FILE_COMPLETE_LABEL,
    FILE_SIGN_FIELD_OPERATE,
    FILE_SIGN_FIELD_LAST
};

struct FileSignRecord
{
    bool isSelected;
    QString filePath;
    QString fileSeContext;
    QString fileCompleteLabel;
};
using FileSignRecordMap = QMap<QString, FileSignRecord>;

class FileSignDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    FileSignDelegate(QObject *parent = 0);
    virtual ~FileSignDelegate(){};
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class FileSignFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    FileSignFilterModel(QObject *parent = nullptr);
    virtual ~FileSignFilterModel(){};

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class FileSignModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    FileSignModel(QObject *parent = nullptr);
    virtual ~FileSignModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void updateData(const FileSignRecordMap &newData);
    void removeData(const QStringList &newData);
    FileSignRecordMap getSelectedData() const;
    FileSignRecordMap getData() const;

private:
    void checkSelectStatus();

signals:
    void stateChanged(Qt::CheckState checkState);

private:
    FileSignRecordMap m_fileRecordMap;
    QStringList m_focusFiles;
};

class FileSignTable : public QTableView
{
    Q_OBJECT
public:
    FileSignTable(QWidget *parent = nullptr);
    virtual ~FileSignTable(){};
    void updateData(const FileSignRecordMap &);
    FileSignRecordMap getData() const;
    QList<QString> getSelectData() const;

public slots:
    // void cleanSelectedData();
    void searchTextChanged(const QString &text);

protected:
    void leaveEvent(QEvent *event);
private slots:
    void mouseEnter(const QModelIndex &index);
    void checkedAllItem(Qt::CheckState checkState);

Q_SIGNALS:
    void dataSizeChanged();

private:
    FileSignFilterModel *m_filterProxy;
    FileSignModel *m_model;
    TableHeaderProxy *m_headerViewProxy;
};

}  // namespace ToolBox
}  // namespace KS
