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

#include "src/ui/tool-box/file-sign/file-sign-table.h"
#include <qt5-log-i.h>
#include <QApplication>
#include <QCheckBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemDelegate>
#include <QJsonDocument>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QToolTip>
#include "file-sign-table.h"
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"
#include "src/ui/common/table/table-header-proxy.h"
#include "src/ui/tp/delegate.h"

namespace KS
{
namespace ToolBox
{
// 文件标记保护列数
#define FILE_SIGN_TABLE_COL 4

FileSignFilterModel::FileSignFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool FileSignFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QString textComb;
    for (auto i = 0; i < FILE_SIGN_TABLE_COL; ++i)
    {
        auto index = sourceModel()->index(sourceRow, i, sourceParent);
        auto text = sourceModel()->data(index).toString();
        RETURN_VAL_IF_TRUE(text.contains(filterRegExp()), true);
    }

    return false;
}

FileSignModel::FileSignModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int FileSignModel::rowCount(const QModelIndex &parent) const
{
    return m_focusFiles.size();
}

int FileSignModel::columnCount(const QModelIndex &parent) const
{
    return FILE_SIGN_TABLE_COL;
}

QVariant FileSignModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

    if (index.row() >= m_focusFiles.size() || index.column() >= FILE_SIGN_TABLE_COL)
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QVariant();
    }

    auto fileSignRecord = m_focusFiles[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (index.column())
        {
        case FileSignField::FILE_SIGN_FIELD_FILE_PATH:
            return QFileInfo(m_fileRecordMap[fileSignRecord].filePath).fileName();
        case FileSignField::FILE_SIGN_FIELD_FILE_SE_CONTEXT:
            return m_fileRecordMap[fileSignRecord].fileSeContext;
        case FileSignField::FILE_SIGN_FIELD_FILE_COMPLETE_LABEL:
            return m_fileRecordMap[fileSignRecord].fileCompleteLabel;
        default:
            break;
        }
    }
    case Qt::EditRole:
    {
        switch (index.column())
        {
        case 0:
            return m_fileRecordMap[fileSignRecord].isSelected;
        case FileSignField::FILE_SIGN_FIELD_FILE_SE_CONTEXT:
            return m_fileRecordMap[fileSignRecord].fileSeContext;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    return QVariant();
}

QVariant FileSignModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    RETURN_VAL_IF_TRUE(orientation == Qt::Orientation::Vertical, QVariant())

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (section)
        {
        case FileSignField::FILE_SIGN_FIELD_FILE_PATH:
            return tr("File Name");
        case FileSignField::FILE_SIGN_FIELD_FILE_SE_CONTEXT:
            return tr("Security Context");
        case FileSignField::FILE_SIGN_FIELD_FILE_COMPLETE_LABEL:
            return tr("Complete Label");
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
    return QVariant();
}

bool FileSignModel::setData(const QModelIndex &index,
                            const QVariant &value,
                            int role)
{
    RETURN_VAL_IF_TRUE(index.column() == FileSignField::FILE_SIGN_FIELD_FILE_SE_CONTEXT, true);
    auto focusFile = m_focusFiles[index.row()];
    m_fileRecordMap[focusFile].isSelected = value.toBool();
    auto updateCellIndex = createIndex(index.row(), FileSignField::FILE_SIGN_FIELD_CHECKBOX);
    emit dataChanged(updateCellIndex, updateCellIndex);

    if (role == Qt::UserRole || role == Qt::EditRole)
    {
        checkSelectStatus();
    }

    return true;
}

Qt::ItemFlags FileSignModel::flags(const QModelIndex &index) const
{
    RETURN_VAL_IF_TRUE(index.column() == 0 || index.column() == 2, Qt::ItemFlag::ItemIsEnabled);

    return Qt::ItemFlag::NoItemFlags;
}

void FileSignModel::updateData(const FileSignRecordMap &fileRecords)
{
    beginResetModel();
    for (const auto &fileRecord : fileRecords)
    {
        const auto &filePath = fileRecord.filePath;
        auto it = m_fileRecordMap.find(filePath);
        if (it == m_fileRecordMap.end())
        {
            m_fileRecordMap.insert(filePath, fileRecord);
            m_focusFiles.append(filePath);
            continue;
        }
        it.value() = std::move(fileRecord);
    }

    endResetModel();
}

void FileSignModel::removeData(const QStringList &filePaths)
{
    beginResetModel();
    for (const auto &filePath : filePaths)
    {
        m_focusFiles.removeOne(filePath);
        m_fileRecordMap.remove(filePath);
    }
    endResetModel();
    checkSelectStatus();
}

FileSignRecordMap FileSignModel::getSelectedData() const
{
    FileSignRecordMap selectedData;
    for (const auto &filePath : m_focusFiles)
    {
        if (!m_fileRecordMap[filePath].isSelected)
        {
            continue;
        }
        selectedData.insert(filePath, m_fileRecordMap[filePath]);
    }
    return selectedData;
}

FileSignRecordMap FileSignModel::getData() const
{
    return m_fileRecordMap;
}

void FileSignModel::checkSelectStatus()
{
    auto state = Qt::Unchecked;
    auto selectedData = getSelectedData();

    if (selectedData.size() >= m_focusFiles.size())
    {
        state = Qt::Checked;
    }
    else if (selectedData.size() > 0)
    {
        state = Qt::PartiallyChecked;
    }

    emit stateChanged(state);
}

FileSignTable::FileSignTable(QWidget *parent)
    : QTableView(parent),
      m_filterProxy(nullptr)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置Model
    m_model = new FileSignModel(this);
    m_headerViewProxy = new TableHeaderProxy(this);
    setHorizontalHeader(m_headerViewProxy);
    setMouseTracking(true);

    m_filterProxy = new FileSignFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);

    setShowGrid(false);
    // 设置Delegate
    setItemDelegate(new KS::TP::Delegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(FileSignField::FILE_SIGN_FIELD_CHECKBOX, 50);
    m_headerViewProxy->resizeSection(FileSignField::FILE_SIGN_FIELD_FILE_PATH, 150);
    m_headerViewProxy->resizeSection(FileSignField::FILE_SIGN_FIELD_FILE_SE_CONTEXT, 300);
    m_headerViewProxy->resizeSection(FileSignField::FILE_SIGN_FIELD_FILE_COMPLETE_LABEL, 150);

    m_headerViewProxy->setStretchLastSection(true);
    m_headerViewProxy->setSectionsMovable(false);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_headerViewProxy->setFixedHeight(24);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
    setMouseTracking(true);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(m_model, &FileSignModel::stateChanged, m_headerViewProxy, &TableHeaderProxy::setCheckState);
    connect(m_headerViewProxy, &TableHeaderProxy::toggled, this, &FileSignTable::checkedAllItem);

    // 希望点击一行中的除了可编辑部分（Security context）的任意位置都可以使得 checkbox 被勾选。
    connect(this, &FileSignTable::clicked, [this](const QModelIndex &index)
            {
                RETURN_IF_TRUE(index.column() == FileSignField::FILE_SIGN_FIELD_FILE_SE_CONTEXT ||
                               index.column() == FileSignField::FILE_SIGN_FIELD_CHECKBOX);
                auto data = getData();
                auto targetData = data.begin() + index.row();
                this->m_model->setData(index, !targetData->isSelected);
            });

    connect(this, &FileSignTable::entered, this, &FileSignTable::mouseEnter);
}

void FileSignTable::updateData(const FileSignRecordMap &newData)
{
    RETURN_IF_TRUE(newData.isEmpty());
    m_model->updateData(newData);
    emit dataSizeChanged();
}

FileSignRecordMap FileSignTable::getData() const
{
    return m_model->getData();
}

void FileSignTable::cleanSelectedData()
{
    m_model->removeData(m_model->getSelectedData().keys());
    emit dataSizeChanged();
}

void FileSignTable::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    m_filterProxy->setFilterFixedString(text);
}

void FileSignTable::mouseEnter(const QModelIndex &index)
{
    RETURN_IF_TRUE(index.column() != FileSignField::FILE_SIGN_FIELD_FILE_COMPLETE_LABEL);
    auto mod = selectionModel()->model()->data(index);
    QToolTip::showText(QCursor::pos(), mod.toString(), this, rect(), 2000);
}

void FileSignTable::checkedAllItem(Qt::CheckState checkState)
{
    for (int i = 0; i < selectionModel()->model()->rowCount(); i++)
    {
        // 取到该行的序号列
        // auto number = selectionModel()->model()->data(model()->index(i, 1)).toInt();
        // auto index = m_model->index(number - 1, 0);
        m_model->setData(m_model->index(i, 0), checkState == Qt::Checked, Qt::CheckStateRole);
    }
}

}  // namespace ToolBox
}  // namespace KS
