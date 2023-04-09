/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * kiran-session-manager is licensed under Mulan PSL v2.
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

#include "src/ui/file-protected/fp-file-table.h"
#include <qt5-log-i.h>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTableView>
#include "sc-i.h"
#include "sc-marcos.h"
#include "src/ui/file_protected_proxy.h"

namespace KS
{
enum FileTableField
{
    FILE_TABLE_FIELD_CHECKBOX,
    FILE_TABLE_FIELD_NUMBER,
    FILE_TABLE_FIELD_FILE_NAME,
    FILE_TABLE_FIELD_FILE_PATH,
    FILE_TABLE_FIELD_ADD_TIME,
    FILE_TABLE_FIELD_LAST
};

FPFilesDelegate::FPFilesDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

FPFilesDelegate::~FPFilesDelegate()
{
    KLOG_DEBUG() << "The FPFilesDelegate is deleted.";
}

QWidget *FPFilesDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    auto editor = new QCheckBox(parent);
    editor->setText(QString());
    return editor;
}

void FPFilesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto value = index.model()->data(index, Qt::EditRole).toBool();

    auto checkbox = static_cast<QCheckBox *>(editor);
    checkbox->setChecked(value);
}

void FPFilesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto checkbox = static_cast<QCheckBox *>(editor);
    model->setData(index, checkbox->isChecked(), Qt::EditRole);
}

void FPFilesDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

FPFilesFilterModel::FPFilesFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool FPFilesFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QString textComb;
    for (auto i = 0; i < FILE_TABLE_FIELD_LAST; ++i)
    {
        auto index = this->sourceModel()->index(sourceRow, i, sourceParent);
        auto text = this->sourceModel()->data(index).toString();
        RETURN_VAL_IF_TRUE(text.contains(this->filterRegExp()), true);
    }

    return false;
}

FPFilesModel::FPFilesModel(QObject *parent) : QAbstractTableModel(parent),
                                              m_allSelected(false)
{
    this->m_fileProtectedProxy = new FileProtectedProxy(SC_DBUS_NAME,
                                                        SC_FILE_PROTECTED_DBUS_OBJECT_PATH,
                                                        QDBusConnection::systemBus(),
                                                        this);

    auto reply = this->m_fileProtectedProxy->GetFiles();
    auto files = reply.value();

    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(files.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files information failed: " << jsonError.errorString();
    }
    else
    {
        auto jsonRoot = jsonDoc.array();

        for (auto iter : jsonRoot)
        {
            auto jsonFile = iter.toObject();
            auto fileInfo = FPFileInfo{.selected = false,
                                       .filePath = jsonFile.value(SCFP_JK_FILE_PATH).toString(),
                                       .addTime = jsonFile.value(SCFP_JK_ADD_TIME).toString()};
            this->m_filesInfo.push_back(fileInfo);
        }
    }

    // TODO: TEST
    for (int i = 0; i < 100; ++i)
    {
        auto fileInfo = FPFileInfo{.selected = false,
                                   .filePath = QString("%1").arg(i),
                                   .addTime = QStringLiteral("2023/04/04")};
        this->m_filesInfo.push_back(fileInfo);
    }
}

int FPFilesModel::rowCount(const QModelIndex &parent) const
{
    return this->m_filesInfo.size();
}

int FPFilesModel::columnCount(const QModelIndex &parent) const
{
    return FileTableField::FILE_TABLE_FIELD_LAST;
}

QVariant FPFilesModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

    if (index.row() >= this->m_filesInfo.size() || index.column() >= FileTableField::FILE_TABLE_FIELD_LAST)
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QVariant();
    }

    auto fileInfo = this->m_filesInfo[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (index.column())
        {
        case FileTableField::FILE_TABLE_FIELD_NUMBER:
            return index.row() + 1;
        case FileTableField::FILE_TABLE_FIELD_FILE_NAME:
            return QFileInfo(fileInfo.filePath).baseName();
        case FileTableField::FILE_TABLE_FIELD_FILE_PATH:
            return fileInfo.filePath;
        case FileTableField::FILE_TABLE_FIELD_ADD_TIME:
            return fileInfo.addTime;
        default:
            break;
        }
    }
    case Qt::EditRole:
    {
        switch (index.column())
        {
        case FileTableField::FILE_TABLE_FIELD_CHECKBOX:
            return QVariant();
        default:
            break;
        }
    }
    default:
        break;
    }

    return QVariant();
}

QVariant FPFilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Vertical)
    {
        return QVariant();
    }
    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (section)
        {
        case FileTableField::FILE_TABLE_FIELD_NUMBER:
            return tr("Number");
        case FileTableField::FILE_TABLE_FIELD_FILE_NAME:
            return tr("File name");
        case FileTableField::FILE_TABLE_FIELD_FILE_PATH:
            return tr("File path");
        case FileTableField::FILE_TABLE_FIELD_ADD_TIME:
            return tr("Add time");
        default:
            break;
        }
    }
    case Qt::EditRole:
    {
        switch (section)
        {
        case FileTableField::FILE_TABLE_FIELD_CHECKBOX:
            return QVariant();
        }
    }
    default:
        break;
    }
    return QVariant();
}

bool FPFilesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() != FileTableField::FILE_TABLE_FIELD_CHECKBOX || role != Qt::EditRole)
    {
        return false;
    }

    this->m_filesInfo[index.row()].selected = value.toBool();
    return true;
}

bool FPFilesModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (section != FileTableField::FILE_TABLE_FIELD_CHECKBOX || role != Qt::EditRole)
    {
        return false;
    }

    this->m_allSelected = value.toBool();
    return true;
}

Qt::ItemFlags FPFilesModel::flags(const QModelIndex &index) const
{
    if (index.column() == FileTableField::FILE_TABLE_FIELD_CHECKBOX)
    {
        return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled;
    }
    return Qt::ItemFlag::NoItemFlags;
}

FPFileTable::FPFileTable(QWidget *parent) : QWidget(parent),
                                            m_view(nullptr),
                                            m_filterProxy(nullptr)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    this->m_view = new QTableView(this);
    this->m_view->setObjectName("m_fpFileTableView");
    this->m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置Model
    auto fileModel = new FPFilesModel(this->m_view);
    this->m_filterProxy = new FPFilesFilterModel(this->m_view);
    this->m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(fileModel));
    this->m_view->setModel(this->m_filterProxy);
    this->m_view->setShowGrid(false);

    // 设置Delegate
    this->m_view->setItemDelegateForColumn(FILE_TABLE_FIELD_CHECKBOX, new FPFilesDelegate(this->m_view));

    // 设置表头
    auto tableHeader = this->m_view->horizontalHeader();
    tableHeader->resizeSection(FileTableField::FILE_TABLE_FIELD_CHECKBOX, 50);
    tableHeader->resizeSection(FileTableField::FILE_TABLE_FIELD_NUMBER, 100);
    tableHeader->resizeSection(FileTableField::FILE_TABLE_FIELD_FILE_NAME, 150);
    tableHeader->resizeSection(FileTableField::FILE_TABLE_FIELD_FILE_PATH, 500);
    tableHeader->setStretchLastSection(true);
    tableHeader->setSectionsMovable(false);
    tableHeader->setDefaultAlignment(Qt::AlignLeft);

    layout->addWidget(this->m_view);
}
}  // namespace KS
