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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "device-list-table.h"
#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include "sc-marcos.h"
#include "src/ui/device/table-filter-model.h"

namespace KS
{
enum DeviceTableField
{
    DEVICE_TABLE_FIELD_NUMBER,
    DEVICE_TABLE_FIELD_NAME,
    DEVICE_TABLE_FIELD_TYPE,
    DEVICE_TABLE_FIELD_ID,
    DEVICE_TABLE_FIELD_INTERFACE,
    DEVICE_TABLE_FIELD_STATUS,
    DEVICE_TABLE_FIELD_PERMISSION,
    DEVICE_TABLE_FIELD_LAST
};

// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

ListDelegate::ListDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

ListDelegate::~ListDelegate()
{
}

void ListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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

    //绘制编辑列:字体样式,字体颜色
    if (index.column() == DEVICE_TABLE_FIELD_PERMISSION)
    {
        QStyleOptionViewItem viewOption(option);
        viewOption.palette.setColor(QPalette::Text, QColor(46, 179, 255));

        QFont font;
        font.setUnderline(true);
        painter->setFont(font);
        painter->setBrush(QBrush(QColor(46, 179, 255)));
        QApplication::style()->drawItemText(painter,
                                            option.rect,
                                            Qt::AlignLeft | Qt::AlignVCenter,
                                            viewOption.palette,
                                            true,
                                            tr("Edit"),
                                            QPalette::Text);
    }
    //TODO:绘制状态列:根据状态显示字体颜色
    else
    {
        this->QStyledItemDelegate::paint(painter, option, index);
    }
}

//DeviceListModel::DeviceListModel(QObject *parent) : QStandardItemModel(parent)
//{
//    auto deviceInfo = DeviceInfo{.number = 1,
//                                 .name = "1",
//                                 .type = 1,
//                                 .id = "1",
//                                 .interface = 1,
//                                 .status = 1,
//                                 .permission = 1};
//    m_deviceInfo.append(deviceInfo);
//    //    this->m_fileProtectedProxy = new FileProtectedProxy(SC_DBUS_NAME,
//    //                                                        SC_FILE_PROTECTED_DBUS_OBJECT_PATH,
//    //                                                        QDBusConnection::systemBus(),
//    //                                                        this);

//    //    auto reply = this->m_fileProtectedProxy->GetFiles();
//    //    auto files = reply.value();

//    //    QJsonParseError jsonError;

//    //    auto jsonDoc = QJsonDocument::fromJson(files.toUtf8(), &jsonError);
//    //    if (jsonDoc.isNull())
//    //    {
//    //        KLOG_WARNING() << "Parser files information failed: " << jsonError.errorString();
//    //    }
//    //    else
//    //    {
//    //        auto jsonRoot = jsonDoc.array();

//    //        for (auto iter : jsonRoot)
//    //        {
//    //            auto jsonFile = iter.toObject();
//    //            auto fileInfo = FPFileInfo{.selected = false,
//    //                                       .filePath = jsonFile.value(SCFP_JK_FILE_PATH).toString(),
//    //                                       .addTime = jsonFile.value(SCFP_JK_ADD_TIME).toString()};
//    //            this->m_filesInfo.push_back(fileInfo);
//    //        }
//    //    }

//    //    // TODO: TEST
//    //    for (int i = 0; i < 100; ++i)
//    //    {
//    //        auto fileInfo = FPFileInfo{.selected = false,
//    //                                   .filePath = QString("%1").arg(i),
//    //                                   .addTime = QStringLiteral("2023/04/04")};
//    //        this->m_filesInfo.push_back(fileInfo);
//    //    }
//}

//int DeviceListModel::rowCount(const QModelIndex &parent) const
//{
//    return this->m_deviceInfo.size();
//}

//int DeviceListModel::columnCount(const QModelIndex &parent) const
//{
//    return DeviceTableField::DEVICE_TABLE_FIELD_LAST;
//}

//QVariant DeviceListModel::data(const QModelIndex &index, int role) const
//{
//    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

//    if (index.row() >= this->m_deviceInfo.size() || index.column() >= index.model()->columnCount())
//    {
//        KLOG_WARNING() << "The index exceeds range limit.";
//        return QVariant();
//    }

//    auto deviceInfo = this->m_deviceInfo[index.row()];

//    switch (role)
//    {
//    case Qt::DisplayRole:
//    {
//        switch (index.column())
//        {
//        case DeviceTableField::DEVICE_TABLE_FIELD_NUMBER:
//            return index.row() + 1;
//        case DeviceTableField::DEVICE_TABLE_FIELD_NAME:
//            return deviceInfo.name;
//        case DeviceTableField::DEVICE_TABLE_FIELD_TYPE:
//            return deviceInfo.type;
//        case DeviceTableField::DEVICE_TABLE_FIELD_ID:
//            return deviceInfo.id;
//        case DeviceTableField::DEVICE_TABLE_FIELD_INTERFACE:
//            return deviceInfo.interface;
//        case DeviceTableField::DEVICE_TABLE_FIELD_STATUS:
//            return deviceInfo.status;
//        case DeviceTableField::DEVICE_TABLE_FIELD_PERMISSION:
//            return deviceInfo.permission;
//        default:
//            break;
//        }
//    }
//    default:
//        break;
//    }

//    return QVariant();
//}

//QVariant DeviceListModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    QStringList head;
//    head << tr("Number")
//         << tr("Device Name")
//         << tr("Device Type")
//         << tr("Device Id")
//         << tr("Device Interface")
//         << tr("Device Status")
//         << tr("Device Permission");

//    if (orientation == Qt::Orientation::Vertical)
//    {
//        return QVariant();
//    }

//    if (role == Qt::DisplayRole)
//    {
//        switch (section)
//        {
//        case DeviceTableField::DEVICE_TABLE_FIELD_NUMBER:
//            return tr("Number");
//        case DeviceTableField::DEVICE_TABLE_FIELD_NAME:
//            return tr("Device name");
//        case DeviceTableField::DEVICE_TABLE_FIELD_TYPE:
//            return tr("Device Type");
//        case DeviceTableField::DEVICE_TABLE_FIELD_ID:
//            return tr("Device Id");
//        case DeviceTableField::DEVICE_TABLE_FIELD_INTERFACE:
//            return tr("Device Interface");
//        case DeviceTableField::DEVICE_TABLE_FIELD_STATUS:
//            return tr("Device Status");
//        case DeviceTableField::DEVICE_TABLE_FIELD_PERMISSION:
//            return tr("Device Permission");

//        default:
//            break;
//        }
//    }
//    return QVariant();
//}

//void DeviceListModel::setData(QList<DeviceInfo> deviceInfos)
//{
//}

//bool DeviceListModel::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//    //    if (index.column() != FileTableField::FILE_TABLE_FIELD_CHECKBOX || role != Qt::EditRole)
//    //    {
//    //        return false;
//    //    }

//    //    this->m_deviceInfo[index.row()].selected = value.toBool();
//    //    return true;
//}

DeviceListTable::DeviceListTable(QWidget *parent) : QTableView(parent)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setFocusPolicy(Qt::NoFocus);
    // 设置Model
    m_model = new QStandardItemModel(this);
    this->m_filterProxy = new TableFilterModel(this);
    this->m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    this->setModel(this->m_filterProxy);
    this->setShowGrid(false);

    // 设置Delegate
    m_delegate = new ListDelegate(this);
    this->setItemDelegate(m_delegate);

    // 设置水平行表头
    auto horizontalHeader = this->horizontalHeader();
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader->setStretchLastSection(true);
    horizontalHeader->setSectionsMovable(false);
    horizontalHeader->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader->setFixedHeight(24);
    horizontalHeader->setDefaultAlignment(Qt::AlignVCenter);

    setHeaderSections(QStringList() << tr("Number")
                                    << tr("Device Name")
                                    << tr("Device Type")
                                    << tr("Device Id")
                                    << tr("Device Interface")
                                    << tr("Device Status")
                                    << tr("Device Permission"));

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
}

void DeviceListTable::setHeaderSections(QStringList sections)
{
    for (int i = 0; i < sections.size(); i++)
    {
        QStandardItem *headItem = new QStandardItem(sections.at(i));
        headItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_model->setHorizontalHeaderItem(i, headItem);
    }
}

void DeviceListTable::setData(const QList<DeviceInfo> &infos)
{
    RETURN_IF_TRUE(infos.isEmpty());
    m_model->removeRows(0, m_model->rowCount());

    m_model->setColumnCount(DeviceTableField::DEVICE_TABLE_FIELD_LAST);
    m_model->setRowCount(infos.size());

    int row = 0;
    for (int i = 0; i < infos.size(); i++)
    {
        auto deviceInfo = infos.at(i);
        m_model->setData(m_model->index(row, DeviceTableField::DEVICE_TABLE_FIELD_NUMBER), deviceInfo.number);
        m_model->setData(m_model->index(row, DeviceTableField::DEVICE_TABLE_FIELD_NAME), deviceInfo.name);
        m_model->setData(m_model->index(row, DeviceTableField::DEVICE_TABLE_FIELD_TYPE), deviceInfo.type);
        m_model->setData(m_model->index(row, DeviceTableField::DEVICE_TABLE_FIELD_ID), deviceInfo.id);
        m_model->setData(m_model->index(row, DeviceTableField::DEVICE_TABLE_FIELD_INTERFACE), deviceInfo.interface);
        m_model->setData(m_model->index(row, DeviceTableField::DEVICE_TABLE_FIELD_STATUS), deviceInfo.status);
        row++;
    }
}

int DeviceListTable::getColCount()
{
    return m_model->columnCount();
}

int DeviceListTable::getRowCount()
{
    return m_model->rowCount();
}

TableFilterModel *DeviceListTable::getFilterProxy()
{
    return this->m_filterProxy;
}

}  // namespace KS
