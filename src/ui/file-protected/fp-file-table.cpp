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

#include "src/ui/file-protected/fp-file-table.h"
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

// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

FPFilesDelegate::FPFilesDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

FPFilesDelegate::~FPFilesDelegate()
{
    KLOG_DEBUG() << "The FPFilesDelegate is deleted.";
}

void FPFilesDelegate::paint(QPainter *painter,
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

    // TODO: 两个问题：1）样式还存在问题; 2）当editorEvent设置状态改变时，当前函数不会立即调用，会等到鼠标移出窗口后才刷新
    if (index.column() == FileTableField::FILE_TABLE_FIELD_CHECKBOX)
    {
        auto checkboxOption = option;
        this->initStyleOption(&checkboxOption, index);

        QStyleOptionButton checkboxStyle;
        QPixmap pix;
        auto value = index.model()->data(index, Qt::EditRole).toBool();
        pix.load(value ? ":/images/checkbox-checked-normal" : ":/images/checkbox-unchecked-normal");
        checkboxStyle.state = value ? QStyle::State_On : QStyle::State_Off;
        checkboxStyle.state |= QStyle::State_Enabled;
        checkboxStyle.iconSize = QSize(20, 20);
        checkboxStyle.rect = option.rect;
        checkboxStyle.rect.setX(option.rect.x() + 22);

        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        //        style->drawControl(QStyle::CE_CheckBox, &checkboxStyle, painter);
        style->drawItemPixmap(painter, option.rect, Qt::AlignCenter, pix);
    }
    else
    {
        this->QStyledItemDelegate::paint(painter, option, index);
    }
}

bool FPFilesDelegate::editorEvent(QEvent *event,
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

    return this->QStyledItemDelegate::editorEvent(event, model, option, index);
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

FPFilesModel::FPFilesModel(QObject *parent) : QAbstractTableModel(parent)
{
    this->m_fileProtectedProxy = new FileProtectedProxy(SC_DBUS_NAME,
                                                        SC_FILE_PROTECTED_DBUS_OBJECT_PATH,
                                                        QDBusConnection::systemBus(),
                                                        this);

    this->updateInfo();
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
    if (index.column() != FileTableField::FILE_TABLE_FIELD_CHECKBOX)
    {
        return false;
    }

    this->m_filesInfo[index.row()].selected = value.toBool();
    emit dataChanged(index, index);

    if (role == Qt::UserRole || role == Qt::EditRole)
    {
        this->onSingleStateChanged();
    }
    return true;
}

Qt::ItemFlags FPFilesModel::flags(const QModelIndex &index) const
{
    if (index.column() == FileTableField::FILE_TABLE_FIELD_CHECKBOX)
    {
        return Qt::ItemFlag::ItemIsEnabled;
    }
    return Qt::ItemFlag::NoItemFlags;
}

void FPFilesModel::updateInfo()
{
    beginResetModel();
    m_filesInfo.clear();
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
        // 后台返回数据需先转为obj后，将obj中的data字段转为arr
        auto jsonDataArray = jsonDoc.object().value(KSS_JSON_KEY_DATA).toArray();
        for (auto jsonData : jsonDataArray)
        {
            auto data = jsonData.toObject();
            auto fileInfo = FPFileInfo{.selected = false,
                                       .fileName = data.value(KSS_JSON_KEY_DATA_FILE_NAME).toString(),
                                       .filePath = data.value(KSS_JSON_KEY_DATA_PATH).toString(),
                                       .addTime = data.value(KSS_JSON_KEY_DATA_ADD_TIME).toString()};
            this->m_filesInfo.push_back(fileInfo);
        }
    }
    endResetModel();
}

QList<FPFileInfo> FPFilesModel::getFPFileInfos()
{
    return m_filesInfo;
}

void FPFilesModel::onStateChanged(Qt::CheckState checkState)
{
    QModelIndex index;
    for (int i = 0; i < rowCount(); ++i)
    {
        index = this->index(i, 0);
        this->setData(index, checkState == Qt::Checked, Qt::CheckStateRole);
    }
}

void FPFilesModel::onSingleStateChanged()
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

    emit this->stateChanged(state);
}

FPFileTable::FPFileTable(QWidget *parent) : QTableView(parent),
                                            m_filterProxy(nullptr)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置Model
    m_model = new FPFilesModel(this);
    m_newHeaderView = new NewHeaderView(this);
    this->setHorizontalHeader(m_newHeaderView);
    this->setMouseTracking(true);
    connect(m_newHeaderView, &NewHeaderView::toggled, m_model, &FPFilesModel::onStateChanged);
    connect(m_model, &FPFilesModel::stateChanged, m_newHeaderView, &NewHeaderView::setCheckState);

    this->m_filterProxy = new FPFilesFilterModel(this);
    this->m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    this->setModel(this->m_filterProxy);
    this->setShowGrid(false);

    // 设置Delegate
    // this->setItemDelegateForColumn(FILE_TABLE_FIELD_CHECKBOX, new FPFilesDelegate(this));
    this->setItemDelegate(new FPFilesDelegate(this));

    // 设置水平行表头
    m_newHeaderView->resizeSection(FileTableField::FILE_TABLE_FIELD_CHECKBOX, 50);
    m_newHeaderView->resizeSection(FileTableField::FILE_TABLE_FIELD_NUMBER, 100);
    m_newHeaderView->resizeSection(FileTableField::FILE_TABLE_FIELD_FILE_NAME, 150);
    m_newHeaderView->resizeSection(FileTableField::FILE_TABLE_FIELD_FILE_PATH, 500);
    m_newHeaderView->setStretchLastSection(true);
    m_newHeaderView->setSectionsMovable(false);
    m_newHeaderView->setDefaultAlignment(Qt::AlignLeft);
    m_newHeaderView->setFixedHeight(24);
    m_newHeaderView->setDefaultAlignment(Qt::AlignVCenter);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);

    connect(this, &FPFileTable::entered, this, &FPFileTable::mouseEnter);
}

void FPFileTable::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    m_filterProxy->setFilterFixedString(text);
}

void FPFileTable::updateInfo()
{
    m_model->updateInfo();
}

QList<FPFileInfo> FPFileTable::getFPFileInfos()
{
    return m_model->getFPFileInfos();
}

void FPFileTable::mouseEnter(const QModelIndex &index)
{
    if (index.column() != FileTableField::FILE_TABLE_FIELD_FILE_PATH)
    {
        return;
    }
    auto mod = this->selectionModel()->model()->data(index);
    QToolTip::showText(QCursor::pos(), mod.toString(), this, this->rect(), 2000);
}

}  // namespace KS
