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

#include "device-table.h"
#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include "include/ksc-marcos.h"
#include "src/ui/device/device-list-delegate.h"
#include "src/ui/device/device-record-delegate.h"
#include "src/ui/device/table-filter-model.h"
namespace KS
{
DeviceTable::DeviceTable(QWidget *parent) : QTableView(parent)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setFocusPolicy(Qt::NoFocus);
    this->setMouseTracking(true);

    // 设置Model
    m_model = new QStandardItemModel(this);
    this->m_filterProxy = new TableFilterModel(this);
    this->m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    this->setModel(this->m_filterProxy);
    this->setShowGrid(false);

    // 设置水平行表头
    auto horizontalHeader = this->horizontalHeader();
    horizontalHeader->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader->setStretchLastSection(true);
    horizontalHeader->setSectionsMovable(false);
    horizontalHeader->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader->setFixedHeight(24);
    horizontalHeader->setDefaultAlignment(Qt::AlignVCenter);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
}

void DeviceTable::setDelegate(QAbstractItemDelegate *delegate)
{
    this->setItemDelegate(delegate);
}

void DeviceTable::setHeaderSections(QStringList sections)
{
    for (int i = 0; i < sections.size(); i++)
    {
        QStandardItem *headItem = new QStandardItem(sections.at(i));
        headItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_model->setHorizontalHeaderItem(i, headItem);
    }
}

void DeviceTable::setData(const QList<DeviceInfo> &infos)
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

void DeviceTable::setData(const QList<RecordsInfo> &infos)
{
    RETURN_IF_TRUE(infos.isEmpty());
    m_model->removeRows(0, m_model->rowCount());

    m_model->setColumnCount(RecordTableField::RECORD_TABLE_FIELD_LAST);
    m_model->setRowCount(infos.size());

    int row = 0;
    for (int i = 0; i < infos.size(); i++)
    {
        auto recordsInfo = infos.at(i);
        m_model->setData(m_model->index(row, RecordTableField::RECORD_TABLE_FIELD_NAME), recordsInfo.name);
        m_model->setData(m_model->index(row, RecordTableField::RECORD_TABLE_FIELD_TYPE), recordsInfo.name);
        m_model->setData(m_model->index(row, RecordTableField::RECORD_TABLE_FIELD_TIME), recordsInfo.type);
        m_model->setData(m_model->index(row, RecordTableField::RECORD_TABLE_FIELD_STATUS), recordsInfo.status);
        row++;
    }
}

int DeviceTable::getColCount()
{
    return m_model->columnCount();
}

int DeviceTable::getRowCount()
{
    return m_model->rowCount();
}

TableFilterModel *DeviceTable::getFilterProxy()
{
    return this->m_filterProxy;
}

}  // namespace KS
