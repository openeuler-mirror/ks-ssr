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

// QWidget *FPFilesDelegate::createEditor(QWidget *parent,
//                                        const QStyleOptionViewItem &option,
//                                        const QModelIndex &index) const
// {
//     // switch (index.column())
//     // {
//     // case FileTableField::FILE_TABLE_FIELD_CHECKBOX:
//     // {
//     //     auto editor = new QCheckBox(parent);
//     //     editor->setText(QString());
//     //     return editor;
//     // }
//     // default:
//     //     break;
//     // }
//     return nullptr;
// }

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
        auto value = index.model()->data(index, Qt::EditRole).toBool();
        KLOG_INFO() << "value: " << value;
        checkboxStyle.state = value ? QStyle::State_On : QStyle::State_Off;
        checkboxStyle.state |= QStyle::State_Enabled;
        checkboxStyle.iconSize = QSize(20, 20);
        checkboxStyle.rect = option.rect;

        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_CheckBox, &checkboxStyle, painter);
        // style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkboxStyle, painter);
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

// void FPFilesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
// {
//     switch (index.column())
//     {
//     case FileTableField::FILE_TABLE_FIELD_CHECKBOX:
//     {
//         auto value = index.model()->data(index, Qt::EditRole).toBool();
//         auto checkbox = static_cast<QCheckBox *>(editor);
//         checkbox->setChecked(value);
//     }
//     default:
//         break;
//     }
// }

// void FPFilesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
// {
//     switch (index.column())
//     {
//     case FileTableField::FILE_TABLE_FIELD_CHECKBOX:
//     {
//         auto checkbox = static_cast<QCheckBox *>(editor);
//         model->setData(index, checkbox->isChecked(), Qt::EditRole);
//     }
//     default:
//         break;
//     }
// }

// void FPFilesDelegate::updateEditorGeometry(QWidget *editor,
//                                            const QStyleOptionViewItem &option,
//                                            const QModelIndex &index) const
// {
//     editor->setGeometry(option.rect);
// }

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
    // if (index.column() == FileTableField::FILE_TABLE_FIELD_CHECKBOX)
    // {
    //     return Qt::ItemFlag::ItemIsEnabled;
    // }
    return Qt::ItemFlag::NoItemFlags;
}

FPFileTable::FPFileTable(QWidget *parent) : QTableView(parent),
                                            m_filterProxy(nullptr)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置Model
    auto fileModel = new FPFilesModel(this);
    this->m_filterProxy = new FPFilesFilterModel(this);
    this->m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(fileModel));
    this->setModel(this->m_filterProxy);
    this->setShowGrid(false);

    // 设置Delegate
    // this->setItemDelegateForColumn(FILE_TABLE_FIELD_CHECKBOX, new FPFilesDelegate(this));
    this->setItemDelegate(new FPFilesDelegate(this));

    // 设置水平行表头
    auto horizontalHeader = this->horizontalHeader();
    horizontalHeader->resizeSection(FileTableField::FILE_TABLE_FIELD_CHECKBOX, 50);
    horizontalHeader->resizeSection(FileTableField::FILE_TABLE_FIELD_NUMBER, 100);
    horizontalHeader->resizeSection(FileTableField::FILE_TABLE_FIELD_FILE_NAME, 150);
    horizontalHeader->resizeSection(FileTableField::FILE_TABLE_FIELD_FILE_PATH, 500);
    horizontalHeader->setStretchLastSection(true);
    horizontalHeader->setSectionsMovable(false);
    horizontalHeader->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader->setFixedHeight(24);
    horizontalHeader->setDefaultAlignment(Qt::AlignVCenter);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
}

}  // namespace KS
