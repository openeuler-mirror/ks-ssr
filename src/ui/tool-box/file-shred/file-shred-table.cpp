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

#include "src/ui/tool-box/file-shred/file-shred-table.h"
// #include <stdio.h>
#include <QApplication>
#include <QCheckBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTableView>
#include <QToolTip>
#include "lib/base/notification-wrapper.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/common/table/table-header-proxy.h"
#include "src/ui/toolbox_dbus_proxy.h"
#include "ssr-i.h"

namespace KS
{
namespace ToolBox
{
enum FileShredTableField
{
    FILE_SHRED_TABLE_FIELD_CHECKBOX,
    FILE_SHRED_TABLE_FIELD_NUMBER,
    FILE_SHRED_TABLE_FIELD_FILE_NAME,
    FILE_SHRED_TABLE_FIELD_FILE_PATH,
    FILE_SHRED_TABLE_FIELD_LAST
};

// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

FileShredDelegate::FileShredDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

FileShredDelegate::~FileShredDelegate()
{
    KLOG_DEBUG() << "The FileShredDelegate is deleted.";
}

void FileShredDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
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

    if (index.column() == FileShredTableField::FILE_SHRED_TABLE_FIELD_CHECKBOX)
    {
        auto checkboxOption = option;
        initStyleOption(&checkboxOption, index);

        QStyleOptionButton checkboxStyle;
        QPixmap pixmap;
        auto value = index.model()->data(index, Qt::EditRole).toBool();
        pixmap.load(value ? ":/images/checkbox-checked-normal" : ":/images/checkbox-unchecked-normal");
        checkboxStyle.state = value ? QStyle::State_On : QStyle::State_Off;
        checkboxStyle.state |= QStyle::State_Enabled;
        checkboxStyle.iconSize = QSize(20, 20);
        checkboxStyle.rect = option.rect;
        checkboxStyle.rect.setX(option.rect.x() + 22);

        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        //        style->drawControl(QStyle::CE_CheckBox, &checkboxStyle, painter);
        style->drawItemPixmap(painter, option.rect, Qt::AlignCenter, pixmap);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool FileShredDelegate::editorEvent(QEvent *event,
                                    QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    auto docorationRect = option.rect;
    auto mouseEvent = static_cast<QMouseEvent *>(event);

    if (event->type() == QEvent::MouseButtonPress &&
        docorationRect.contains(mouseEvent->pos()) &&
        index.column() == FileShredTableField::FILE_SHRED_TABLE_FIELD_CHECKBOX)
    {
        auto value = model->data(index, Qt::EditRole).toBool();
        model->setData(index, !value, Qt::EditRole);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

FileShredFilterModel::FileShredFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool FileShredFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    for (auto i = 0; i < FILE_SHRED_TABLE_FIELD_LAST; ++i)
    {
        auto index = sourceModel()->index(sourceRow, i, sourceParent);
        auto text = sourceModel()->data(index).toString();
        RETURN_VAL_IF_TRUE(text.contains(filterRegExp()), true);
    }

    return false;
}

FileShredModel::FileShredModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_infos = {};
}

int FileShredModel::rowCount(const QModelIndex &parent) const
{
    return m_infos.size();
}

int FileShredModel::columnCount(const QModelIndex &parent) const
{
    return FileShredTableField::FILE_SHRED_TABLE_FIELD_LAST;
}

QVariant FileShredModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

    if (index.row() >= m_infos.size() || index.column() >= FileShredTableField::FILE_SHRED_TABLE_FIELD_LAST)
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QVariant();
    }

    auto info = m_infos[index.row()];
    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (index.column())
        {
        case FileShredTableField::FILE_SHRED_TABLE_FIELD_NUMBER:
            return index.row() + 1;
        case FileShredTableField::FILE_SHRED_TABLE_FIELD_FILE_NAME:
            return info.fileName;
        case FileShredTableField::FILE_SHRED_TABLE_FIELD_FILE_PATH:
            return info.filePath;
        default:
            break;
        }
        break;
    }
    case Qt::EditRole:
    {
        switch (index.column())
        {
        case FileShredTableField::FILE_SHRED_TABLE_FIELD_CHECKBOX:
            return info.selected;
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

QVariant FileShredModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        case FileShredTableField::FILE_SHRED_TABLE_FIELD_NUMBER:
            return tr("Number");
        case FileShredTableField::FILE_SHRED_TABLE_FIELD_FILE_NAME:
            return tr("File name");
        case FileShredTableField::FILE_SHRED_TABLE_FIELD_FILE_PATH:
            return tr("File path");
        default:
            break;
        }
        break;
    }
    case Qt::EditRole:
    {
        switch (section)
        {
        case FileShredTableField::FILE_SHRED_TABLE_FIELD_CHECKBOX:
            return QVariant();
        }
        break;
    }
    default:
        break;
    }
    return QVariant();
}

bool FileShredModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    RETURN_VAL_IF_TRUE(index.column() != FileShredTableField::FILE_SHRED_TABLE_FIELD_CHECKBOX, false);

    m_infos[index.row()].selected = value.toBool();
    emit dataChanged(index, index);

    if (role == Qt::UserRole || role == Qt::EditRole)
    {
        checkSelectStatus();
    }
    return true;
}

Qt::ItemFlags FileShredModel::flags(const QModelIndex &index) const
{
    RETURN_VAL_IF_TRUE(index.column() == FileShredTableField::FILE_SHRED_TABLE_FIELD_CHECKBOX, Qt::ItemFlag::ItemIsEnabled);
    return Qt::ItemFlag::NoItemFlags;
}

void FileShredModel::checkSelectStatus()
{
    auto state = Qt::Unchecked;
    int selectCount = 0;
    for (int i = 0; i < m_infos.size(); ++i)
    {
        if (m_infos[i].selected)
        {
            ++selectCount;
        }
    }

    if (selectCount >= m_infos.size())
    {
        state = Qt::Checked;
    }
    else if (selectCount > 0)
    {
        state = Qt::PartiallyChecked;
    }

    emit stateChanged(state);
}

int FileShredModel::getFileShredInfosSize()
{
    return m_infos.size();
}

QStringList FileShredModel::getCheckedPath()
{
    QStringList list;
    for (auto info : m_infos)
    {
        CONTINUE_IF_TRUE(!info.selected);
        list << info.filePath;
    }
    return list;
}

void FileShredModel::addFiles(const QStringList &paths)
{
    beginResetModel();
    SCOPE_EXIT(
        {
            endResetModel();
        });

    // 取出路径列表，用于判断是否有重复文件
    QStringList list;
    for (auto info : m_infos)
    {
        list << info.filePath;
    }

    for (auto path : paths)
    {
        CONTINUE_IF_TRUE(path.isEmpty());
        CONTINUE_IF_TRUE(list.contains(path));
        QFileInfo fileInfo(path);

        auto info = FileShredInfo{.selected = false,
                                  .fileName = fileInfo.fileName(),
                                  .filePath = path};
        m_infos << info;
    }

    emit tableUpdated(m_infos.size());
}

void FileShredModel::delFiles()
{
    beginResetModel();
    SCOPE_EXIT(
        {
            endResetModel();
        });
    auto i = -1;
    for (auto info : m_infos)
    {
        i++;
        CONTINUE_IF_TRUE(!info.selected);
        m_infos.removeAt(i);
        i--;
    }

    checkSelectStatus();
    emit tableUpdated(m_infos.size());
}

FileShredTable::FileShredTable(QWidget *parent)
    : QTableView(parent),
      m_filterProxy(nullptr)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_dbusProxy = new ToolBoxDbusProxy(SSR_DBUS_NAME,
                                       SSR_TOOL_BOX_DBUS_OBJECT_PATH,
                                       QDBusConnection::systemBus(),
                                       this);
    connect(m_dbusProxy, &ToolBoxDbusProxy::HazardDetected, this, [](uint type, const QString &alertMessage)
            {
                // TODO 区分类型弹窗？
                Q_UNUSED(type)
                Notify::NOTIFY_ERROR(alertMessage.toUtf8());
            });
    initTable();
}

void FileShredTable::setSearchText(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;
    m_filterProxy->setFilterFixedString(text);
}

void KS::ToolBox::FileShredTable::addFiles(const QStringList &paths)
{
    RETURN_IF_TRUE(paths.isEmpty());
    m_model->addFiles(paths);
}

void FileShredTable::delFiles()
{
    if (m_model->getCheckedPath().isEmpty())
    {
        POPUP_MESSAGE_DIALOG(tr("Please selecte files."))
        return;
    }
    m_model->delFiles();
}

void FileShredTable::shredFiles()
{
    auto checkedPath = m_model->getCheckedPath();
    if (checkedPath.isEmpty())
    {
        POPUP_MESSAGE_DIALOG(tr("Please selecte files."))
        return;
    }
    auto reply = m_dbusProxy->ShredFile(checkedPath);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    RETURN_IF_TRUE(reply.isError());
    m_model->delFiles();
}

void FileShredTable::initTable()
{
    // 设置Model
    m_model = new FileShredModel(this);
    m_headerViewProxy = new TableHeaderProxy(this);
    setHorizontalHeader(m_headerViewProxy);
    setMouseTracking(true);
    connect(m_headerViewProxy, &TableHeaderProxy::toggled, this, &FileShredTable::checkedAllItem);
    connect(m_model, &FileShredModel::stateChanged, m_headerViewProxy, &TableHeaderProxy::setCheckState);
    connect(m_model, &FileShredModel::tableUpdated, this, &FileShredTable::tableUpdated);

    m_filterProxy = new FileShredFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);
    setShowGrid(false);

    // 设置Delegate
    setItemDelegate(new FileShredDelegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(FileShredTableField::FILE_SHRED_TABLE_FIELD_CHECKBOX, 50);
    m_headerViewProxy->resizeSection(FileShredTableField::FILE_SHRED_TABLE_FIELD_NUMBER, 150);
    m_headerViewProxy->resizeSection(FileShredTableField::FILE_SHRED_TABLE_FIELD_FILE_NAME, 200);
    m_headerViewProxy->resizeSection(FileShredTableField::FILE_SHRED_TABLE_FIELD_FILE_PATH, 300);
    m_headerViewProxy->setStretchLastSection(true);
    m_headerViewProxy->setSectionsMovable(false);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignLeft);
    m_headerViewProxy->setFixedHeight(24);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignVCenter);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
}

int FileShredTable::getFileShredInfosSize()
{
    return m_model->getFileShredInfosSize();
}

void FileShredTable::checkedAllItem(Qt::CheckState checkState)
{
    for (int i = 0; i < selectionModel()->model()->rowCount(); i++)
    {
        // 取到该行的序号列
        auto number = selectionModel()->model()->data(model()->index(i, 1)).toInt();
        auto index = m_model->index(number - 1, 0);
        m_model->setData(index, checkState == Qt::Checked, Qt::CheckStateRole);
    }
}

}  // namespace ToolBox
}  // namespace KS
