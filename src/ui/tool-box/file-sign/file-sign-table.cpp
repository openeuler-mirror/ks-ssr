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
#include "common/ssr-marcos-ui.h"
#include "file-sign-table.h"
#include "include/ssr-i.h"
#include "src/ui/common/table/table-header-proxy.h"
#include "src/ui/tp/delegate.h"

namespace KS
{
namespace ToolBox
{
// 文件标记保护列数
#define FILE_SIGN_TABLE_COL 6
// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

FileSignFilterModel::FileSignFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool FileSignFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
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
        case FileSignField::FILE_SIGN_FIELD_NUMBER:
            return index.row() + 1;
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
        case FileSignField::FILE_SIGN_FIELD_NUMBER:
            return tr("Number");
        case FileSignField::FILE_SIGN_FIELD_FILE_PATH:
            return tr("Object Name");
        case FileSignField::FILE_SIGN_FIELD_FILE_SE_CONTEXT:
            return tr("Security Context");
        case FileSignField::FILE_SIGN_FIELD_FILE_COMPLETE_LABEL:
            return tr("Complete Label");
        case FileSignField::FILE_SIGN_FIELD_OPERATE:
            return tr("Operate");
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
    // auto focusFile = m_focusFiles[index.row()];
    auto indexNumber = this->index(index.row(), FileSignField::FILE_SIGN_FIELD_NUMBER);
    auto number = this->data(indexNumber).toInt();
    auto indexFile = this->index(index.row(), FileSignField::FILE_SIGN_FIELD_FILE_PATH);
    auto fileName = this->data(indexFile).toString().toStdString();

    auto focusFile = (this->getData().begin() + number - 1)->filePath;
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
    RETURN_VAL_IF_TRUE(index.column() == 0, Qt::ItemFlag::ItemIsEnabled);
    return Qt::ItemFlag::NoItemFlags;
}

void FileSignModel::updateData(const FileSignRecordMap &fileRecords)
{
    beginResetModel();
    // 全量更新。
    m_fileRecordMap.clear();
    m_focusFiles.clear();
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
    checkSelectStatus();
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

    if (selectedData.size() == 0)
    {
        state = Qt::Unchecked;
    }
    else if (selectedData.size() >= m_focusFiles.size())
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
    setItemDelegate(new FileSignDelegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(FileSignField::FILE_SIGN_FIELD_CHECKBOX, 50);
    m_headerViewProxy->resizeSection(FileSignField::FILE_SIGN_FIELD_NUMBER, 50);
    m_headerViewProxy->resizeSection(FileSignField::FILE_SIGN_FIELD_FILE_PATH, 150);
    m_headerViewProxy->resizeSection(FileSignField::FILE_SIGN_FIELD_FILE_SE_CONTEXT, 200);
    m_headerViewProxy->resizeSection(FileSignField::FILE_SIGN_FIELD_FILE_COMPLETE_LABEL, 150);
    m_headerViewProxy->resizeSection(FileSignField::FILE_SIGN_FIELD_OPERATE, 100);

    m_headerViewProxy->setStretchLastSection(true);
    m_headerViewProxy->setSectionsMovable(false);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignLeft);
    m_headerViewProxy->setFixedHeight(24);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
    setMouseTracking(true);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(m_model, &FileSignModel::stateChanged, m_headerViewProxy, &TableHeaderProxy::setCheckState);
    connect(m_headerViewProxy, &TableHeaderProxy::toggled, this, &FileSignTable::checkedAllItem);

    connect(this, &FileSignTable::entered, this, &FileSignTable::mouseEnter);
    connect(this, &FileSignTable::entered, this, [this](const QModelIndex &index)
            {
                RETURN_IF_TRUE(!index.isValid());
                RETURN_IF_TRUE(index.column() > m_model->columnCount() || index.row() > m_model->rowCount());
                RETURN_IF_TRUE(index.column() != FileSignField::FILE_SIGN_FIELD_FILE_PATH);
                // 获取完整路径悬浮显示
                auto indexNumber = this->model()->index(index.row(), FileSignField::FILE_SIGN_FIELD_NUMBER);
                auto number = this->model()->data(indexNumber).toInt();
                auto data = getData();
                auto filePath = (data.begin() + number - 1)->filePath;
                QToolTip::showText(QCursor::pos(), filePath, this, rect(), 5000);
            });
    // 编辑列设置鼠标手形
    connect(this, &FileSignTable::entered, this, [this](const QModelIndex &index)
            {
                RETURN_IF_TRUE(!index.isValid());
                RETURN_IF_TRUE(index.column() > m_model->columnCount() || index.row() > m_model->rowCount());
                RETURN_IF_TRUE(index.column() != FileSignField::FILE_SIGN_FIELD_OPERATE);
                setCursor(index.column() == FileSignField::FILE_SIGN_FIELD_OPERATE ? Qt::PointingHandCursor : Qt::ArrowCursor);
            });
}

void FileSignTable::updateData(const FileSignRecordMap &newData)
{
    m_model->updateData(newData);
    emit dataSizeChanged();
}

FileSignRecordMap FileSignTable::getData() const
{
    return m_model->getData();
}

QList<QString> FileSignTable::getSelectData() const
{
    return m_model->getSelectedData().keys();
}

void FileSignTable::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    m_filterProxy->setFilterFixedString(text);
}

void FileSignTable::leaveEvent(QEvent *event)
{
    auto mouseEvent = static_cast<QMouseEvent *>(event);
    // 处理鼠标移出表格事件，将鼠标变为箭头
    if (mouseEvent->type() == QEvent::Leave)
    {
        setCursor(Qt::ArrowCursor);
    }
}

void FileSignTable::mouseEnter(const QModelIndex &index)
{
    RETURN_IF_TRUE(!index.isValid());
    RETURN_IF_TRUE(index.column() > m_model->columnCount() || index.row() > m_model->rowCount());
    // 过滤掉文件名列，不需要悬浮显示完整内容
    RETURN_IF_TRUE(m_model->columnCount() == FileSignField::FILE_SIGN_FIELD_FILE_PATH);
    // 判断内容是否显示完整
    auto itemRect = this->visualRect(index);
    // 计算文本宽度
    QFontMetrics metrics(this->font());
    auto textWidth = metrics.horizontalAdvance(m_model->data(index).toString());
    RETURN_IF_TRUE(textWidth <= itemRect.width())
    auto mod = selectionModel()->model()->data(index);
    QToolTip::showText(QCursor::pos(), mod.toString(), this, rect(), 5000);
}

void FileSignTable::checkedAllItem(Qt::CheckState checkState)
{
    for (int i = 0; i < selectionModel()->model()->rowCount(); i++)
    {
        // 取到该行的序号列
        auto number = selectionModel()->model()->data(model()->index(i, 1)).toInt();
        auto index = m_model->index(number - 1, 0);
        m_model->setData(index, checkState == Qt::Checked, Qt::CheckStateRole);
    }
}

FileSignDelegate::FileSignDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

bool FileSignDelegate::editorEvent(QEvent *event,
                                   QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index)
{
    auto docorationRect = option.rect;
    auto mouseEvent = static_cast<QMouseEvent *>(event);

    if (event->type() == QEvent::MouseButtonPress &&
        docorationRect.contains(mouseEvent->pos()) &&
        index.column() == FileSignField::FILE_SIGN_FIELD_CHECKBOX)
    {
        auto value = model->data(index, Qt::EditRole).toBool();
        model->setData(index, !value, Qt::EditRole);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void FileSignDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QPainterPath path;
    painter->setRenderHint(QPainter::RenderHint::Antialiasing);
    if (index.column() == 0)
    {
        auto rect = option.rect.adjusted(0, 2, TABLE_LINE_RADIUS, -2);
        path.addRoundedRect(rect, TABLE_LINE_RADIUS, TABLE_LINE_RADIUS);
    }
    else if (index.column() == index.model()->columnCount(index.parent()) - 1)
    {
        auto rect = option.rect.adjusted(-TABLE_LINE_RADIUS, 2, 0, -2);
        path.addRoundedRect(rect, TABLE_LINE_RADIUS, TABLE_LINE_RADIUS);
    }
    else
    {
        auto rect = option.rect.adjusted(0, 2, 0, -2);
        path.addRect(rect);
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(57, 57, 57)));
    painter->drawPath(path);
    painter->restore();

    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    // 绘制的文字向右偏移10px，与整体表格风格统一
    auto textRect = option.rect.adjusted(10, 0, 0, 0);
    if (index.column() == FILE_SIGN_FIELD_CHECKBOX)
    {
        auto checkboxOption = option;
        initStyleOption(&checkboxOption, index);

        //        QStyleOptionButton checkboxStyle;
        QPixmap pixmap;
        auto value = index.model()->data(index, Qt::EditRole).toBool();
        pixmap.load(value ? ":/images/checkbox-checked-normal" : ":/images/checkbox-unchecked-normal");
        auto widget = option.widget;
        auto style = widget ? widget->style() : QApplication::style();
        style->drawItemPixmap(painter, option.rect, Qt::AlignCenter, pixmap);
    }
    // 绘制编辑列:字体样式,字体颜色
    else if (index.column() == FILE_SIGN_FIELD_OPERATE)
    {
        viewOption.palette.setColor(QPalette::Text, QColor(46, 179, 255));

        QFont font;
        font.setUnderline(true);
        painter->setFont(font);
        QApplication::style()->drawItemText(painter,
                                            textRect,
                                            Qt::AlignLeft | Qt::AlignVCenter,
                                            viewOption.palette,
                                            true,
                                            tr("Edit"),
                                            QPalette::Text);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

}  // namespace ToolBox
}  // namespace KS
