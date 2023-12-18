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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/ui/fp/file-table.h"
#include <qt5-log-i.h>
#include <stdio.h>
#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTableView>
#include <QToolTip>
#include "src/ui/kss_dbus_proxy.h"
#include "ssr-i.h"
#include "ssr-marcos.h"
#include "src/ui/common/table/header-button-delegate.h"

namespace KS
{
namespace FP
{
#define KSS_JSON_KEY_DATA SSR_KSS_JK_DATA
#define KSS_JSON_KEY_DATA_PATH SSR_KSS_JK_DATA_PATH
#define KSS_JSON_KEY_DATA_FILE_NAME SSR_KSS_JK_DATA_FILE_NAME
#define KSS_JSON_KEY_DATA_ADD_TIME SSR_KSS_JK_DATA_ADD_TIME

enum FileTableField
{
    FILE_TABLE_FIELD_CHECKBOX,
    FILE_TABLE_FIELD_NUMBER,
    FILE_TABLE_FIELD_FILE_NAME,
    FILE_TABLE_FIELD_FILE_PATH,
    FILE_TABLE_FIELD_ADD_TIME,
    FILE_TABLE_FIELD_LAST
};

// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

FilesDelegate::FilesDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

FilesDelegate::~FilesDelegate()
{
    KLOG_DEBUG() << "The FilesDelegate is deleted.";
}

void FilesDelegate::paint(QPainter *painter,
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

    if (index.column() == FileTableField::FILE_TABLE_FIELD_CHECKBOX)
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

bool FilesDelegate::editorEvent(QEvent *event,
                                QAbstractItemModel *model,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index)
{
    auto docorationRect = option.rect;
    auto mouseEvent = static_cast<QMouseEvent *>(event);

    if (event->type() == QEvent::MouseButtonPress &&
        docorationRect.contains(mouseEvent->pos()) &&
        index.column() == FileTableField::FILE_TABLE_FIELD_CHECKBOX)
    {
        auto value = model->data(index, Qt::EditRole).toBool();
        model->setData(index, !value, Qt::EditRole);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

FilesFilterModel::FilesFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool FilesFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QString textComb;
    for (auto i = 0; i < FILE_TABLE_FIELD_LAST; ++i)
    {
        auto index = sourceModel()->index(sourceRow, i, sourceParent);
        auto text = sourceModel()->data(index).toString();
        RETURN_VAL_IF_TRUE(text.contains(filterRegExp()), true);
    }

    return false;
}

FilesModel::FilesModel(QObject *parent) : QAbstractTableModel(parent)
{
    m_fileProtectedProxy = new KSSDbusProxy(SSR_DBUS_NAME,
                                            SSR_KSS_INIT_DBUS_OBJECT_PATH,
                                            QDBusConnection::systemBus(),
                                            this);
    connect(m_fileProtectedProxy, &KSSDbusProxy::ProtectedFilesChange, this, &FilesModel::updateRecord);

    updateRecord();
}

int FilesModel::rowCount(const QModelIndex &parent) const
{
    return m_filesInfo.size();
}

int FilesModel::columnCount(const QModelIndex &parent) const
{
    return FileTableField::FILE_TABLE_FIELD_LAST;
}

QVariant FilesModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

    if (index.row() >= m_filesInfo.size() || index.column() >= FileTableField::FILE_TABLE_FIELD_LAST)
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QVariant();
    }

    auto fileInfo = m_filesInfo[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (index.column())
        {
        case FileTableField::FILE_TABLE_FIELD_NUMBER:
            return index.row() + 1;
        case FileTableField::FILE_TABLE_FIELD_FILE_NAME:
            return fileInfo.fileName;
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
            return fileInfo.selected;
        default:
            break;
        }
    }
    default:
        break;
    }

    return QVariant();
}

QVariant FilesModel::headerData(int section, Qt::Orientation orientation, int role) const
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

bool FilesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() != FileTableField::FILE_TABLE_FIELD_CHECKBOX)
    {
        return false;
    }

    m_filesInfo[index.row()].selected = value.toBool();
    emit dataChanged(index, index);

    if (role == Qt::UserRole || role == Qt::EditRole)
    {
        checkSelectStatus();
    }
    return true;
}

Qt::ItemFlags FilesModel::flags(const QModelIndex &index) const
{
    if (index.column() == FileTableField::FILE_TABLE_FIELD_CHECKBOX)
    {
        return Qt::ItemFlag::ItemIsEnabled;
    }
    return Qt::ItemFlag::NoItemFlags;
}

void FilesModel::updateRecord()
{
    beginResetModel();
    SCOPE_EXIT({
        endResetModel();
    });

    m_filesInfo.clear();
    // 刷新时checkbox状态清空
    emit stateChanged(Qt::Unchecked);

    auto reply = m_fileProtectedProxy->GetProtectedFiles();
    reply.waitForFinished();
    auto files = reply.value();

    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(files.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files information failed: " << jsonError.errorString();
        return;
    }

    // 后台返回数据需先转为obj后，将obj中的data字段转为arr
    auto jsonDataArray = jsonDoc.object().value(KSS_JSON_KEY_DATA).toArray();
    // 倒序排序
    auto jsonData = jsonDataArray.end();
    while (jsonData != jsonDataArray.begin())
    {
        jsonData--;
        auto data = jsonData->toObject();
        auto a = data.value(KSS_JSON_KEY_DATA_FILE_NAME).toString();
        auto fileInfo = FPFileInfo{.selected = false,
                                   .fileName = data.value(KSS_JSON_KEY_DATA_FILE_NAME).toString(),
                                   .filePath = data.value(KSS_JSON_KEY_DATA_PATH).toString(),
                                   .addTime = data.value(KSS_JSON_KEY_DATA_ADD_TIME).toString()};
        m_filesInfo.push_back(fileInfo);
    }

    emit filesUpdate(m_filesInfo.size());
}

QList<FPFileInfo> FilesModel::getFPFileInfos()
{
    return m_filesInfo;
}

void FilesModel::checkSelectStatus()
{
    auto state = Qt::Unchecked;
    int selectCount = 0;
    for (int i = 0; i < m_filesInfo.size(); ++i)
    {
        if (m_filesInfo[i].selected)
        {
            ++selectCount;
        }
    }

    if (selectCount >= m_filesInfo.size())
    {
        state = Qt::Checked;
    }
    else if (selectCount > 0)
    {
        state = Qt::PartiallyChecked;
    }

    emit stateChanged(state);
}

FileTable::FileTable(QWidget *parent) : QTableView(parent),
                                        m_filterProxy(nullptr)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置Model
    m_model = new FilesModel(this);
    m_headerViewProxy = new TableHeaderProxy(this);
    setHorizontalHeader(m_headerViewProxy);
    setMouseTracking(true);
    connect(m_headerViewProxy, &TableHeaderProxy::toggled, this, &FileTable::checkedAllItem);
    connect(m_model, &FilesModel::stateChanged, m_headerViewProxy, &TableHeaderProxy::setCheckState);
    connect(m_model, &FilesModel::filesUpdate, this, &FileTable::filesUpdate);

    m_filterProxy = new FilesFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);
    setShowGrid(false);

    // 设置Delegate
    // setItemDelegateForColumn(FILE_TABLE_FIELD_CHECKBOX, new FilesDelegate(this));
    setItemDelegate(new FilesDelegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(FileTableField::FILE_TABLE_FIELD_CHECKBOX, 50);
    m_headerViewProxy->resizeSection(FileTableField::FILE_TABLE_FIELD_NUMBER, 100);
    m_headerViewProxy->resizeSection(FileTableField::FILE_TABLE_FIELD_FILE_NAME, 200);
    m_headerViewProxy->resizeSection(FileTableField::FILE_TABLE_FIELD_FILE_PATH, 400);
    m_headerViewProxy->setStretchLastSection(true);
    m_headerViewProxy->setSectionsMovable(false);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignLeft);
    m_headerViewProxy->setFixedHeight(24);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignVCenter);
    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);

    connect(this, &FileTable::entered, this, &FileTable::mouseEnter);
}

void FileTable::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    m_filterProxy->setFilterFixedString(text);
}

QList<FPFileInfo> FileTable::getFPFileInfos()
{
    return m_model->getFPFileInfos();
}

void FileTable::mouseEnter(const QModelIndex &index)
{
    RETURN_IF_TRUE(index.column() != FileTableField::FILE_TABLE_FIELD_FILE_PATH)
    auto mod = selectionModel()->model()->data(index);
    QToolTip::showText(QCursor::pos(), mod.toString(), this, rect(), 2000);
}

void FileTable::checkedAllItem(Qt::CheckState checkState)
{
    for (int i = 0; i < selectionModel()->model()->rowCount(); i++)
    {
        // 取到该行的序号列
        auto number = selectionModel()->model()->data(model()->index(i, 1)).toInt();
        auto index = m_model->index(number - 1, 0);
        m_model->setData(index, checkState == Qt::Checked, Qt::CheckStateRole);
    }
}

}  // namespace FP
}  // namespace KS
