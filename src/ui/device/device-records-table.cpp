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

#include "device-records-table.h"
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
enum RecordTableField
{
    RECORD_TABLE_FIELD_NAME,
    RECORD_TABLE_FIELD_TYPE,
    RECORD_TABLE_FIELD_TIME,
    RECORD_TABLE_FIELD_STATUS,
    RECORD_TABLE_FIELD_LAST
};

// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

RecordDelegate::RecordDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

RecordDelegate::~RecordDelegate()
{
}

void RecordDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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

    this->QStyledItemDelegate::paint(painter, option, index);
}

DeviceRecordsTable::DeviceRecordsTable(QWidget *parent) : QTableView(parent)
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
    m_delegate = new RecordDelegate(this);
    this->setItemDelegate(m_delegate);

    // 设置水平行表头
    auto horizontalHeader = this->horizontalHeader();
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader->setStretchLastSection(true);
    horizontalHeader->setSectionsMovable(false);
    horizontalHeader->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader->setFixedHeight(24);
    horizontalHeader->setDefaultAlignment(Qt::AlignVCenter);

    setHeaderSections(QStringList() << tr("Device Name")
                                    << tr("Device Type")
                                    << tr("Device Time")
                                    << tr("Device Status"));

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
}

void DeviceRecordsTable::setHeaderSections(QStringList sections)
{
    for (int i = 0; i < sections.size(); i++)
    {
        QStandardItem *headItem = new QStandardItem(sections.at(i));
        headItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_model->setHorizontalHeaderItem(i, headItem);
    }
}

void DeviceRecordsTable::setData(const QList<RecordsInfo> &infos)
{
    RETURN_IF_TRUE(infos.isEmpty());
    m_model->removeRows(0, m_model->rowCount());

    m_model->setColumnCount(RecordTableField::RECORD_TABLE_FIELD_LAST);
    m_model->setRowCount(infos.size());

    int row = 0;
    for (int i = 0; i < infos.size(); i++)
    {
        auto info = infos.at(i);
        m_model->setData(m_model->index(row, RecordTableField::RECORD_TABLE_FIELD_NAME), info.name);
        m_model->setData(m_model->index(row, RecordTableField::RECORD_TABLE_FIELD_TYPE), info.name);
        m_model->setData(m_model->index(row, RecordTableField::RECORD_TABLE_FIELD_TIME), info.type);
        m_model->setData(m_model->index(row, RecordTableField::RECORD_TABLE_FIELD_STATUS), info.status);
        row++;
    }
}

int DeviceRecordsTable::getColCount()
{
    return m_model->columnCount();
}

int DeviceRecordsTable::getRowCount()
{
    return m_model->rowCount();
}

TableFilterModel *DeviceRecordsTable::getFilterProxy()
{
    return this->m_filterProxy;
}

}  // namespace KS
