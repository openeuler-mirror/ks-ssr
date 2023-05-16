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

#include "device-log-table.h"
#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include "include/ksc-i.h"
#include "include/ksc-marcos.h"
#include "src/ui/device/table-filter-model.h"
namespace KS
{
// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

DeviceLogDelegate::DeviceLogDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

DeviceLogDelegate::~DeviceLogDelegate()
{
}

void DeviceLogDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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

    //TODO:绘制状态列:根据状态显示字体颜色

    this->QStyledItemDelegate::paint(painter, option, index);
}

DeviceLogTable::DeviceLogTable(QWidget *parent) : QTableView(parent),
                                                  m_filterProxy(nullptr),
                                                  m_model(nullptr),
                                                  m_deviceManagerProxy(nullptr)
{
    m_deviceManagerProxy = new DeviceManagerProxy(KSC_DBUS_NAME,
                                                  KSC_DEVICE_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);
    initTable();
}

void DeviceLogTable::initTable()
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

    // 设置代理
    this->setItemDelegate(new DeviceLogDelegate(this));

    // 设置水平行表头
    auto horizontalHeader = this->horizontalHeader();
    horizontalHeader->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader->setStretchLastSection(true);
    horizontalHeader->setSectionsMovable(false);
    horizontalHeader->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader->setFixedHeight(24);
    horizontalHeader->setDefaultAlignment(Qt::AlignVCenter);
    setHeaderSections(QStringList() << tr("Device Name")
                                    << tr("Device Type")
                                    << tr("Device Time")
                                    << tr("Device Status"));
    horizontalHeader->resizeSection(LogTableField::LOG_TABLE_FIELD_NAME, 200);
    horizontalHeader->resizeSection(LogTableField::LOG_TABLE_FIELD_TIME, 200);
    horizontalHeader->resizeSection(LogTableField::LOG_TABLE_FIELD_TYPE, 200);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
}

void DeviceLogTable::update()
{
    //    m_recordsInfo.clear();
    //    auto reply = m_deviceManagerProxy->GetRecords();
    //    reply.waitForFinished();
    //    auto recordJson = reply.value();
    //    KLOG_DEBUG() << recordJson;

    //    QJsonParseError jsonError;

    //    auto jsonDoc = QJsonDocument::fromJson(recordJson.toUtf8(), &jsonError);
    //    if (jsonDoc.isNull())
    //    {
    //        KLOG_WARNING() << "Parser files information failed: " << jsonError.errorString();
    //    }
    //    else
    //    {
    //        int count = 1;
    //        // 后台返回数据需先转为obj后，将obj中的data字段转为arr
    //        auto jsonDataArray = jsonDoc.array();
    //        for (auto jsonData : jsonDataArray)
    //        {
    //            auto data = jsonData.toObject();

    //            auto recordInfo = RecordInfo{.name = data.value(KSC_DEVICE_KEY_NAME).toString(),
    //                                         .type = (DeviceType)data.value(KSC_DEVICE_KEY_TYPE).toInt(),
    //                                         .time = data.value(KSC_DEVICE_KEY_ID).toString(),
    //                                         .state = (DeviceState)data.value(KSC_DEVICE_KEY_STATE).toInt()};
    //            m_recordsInfo.push_back(recordInfo);
    //            count++;
    //        }

    //        setData(m_recordsInfo);
    //    }
}

void DeviceLogTable::setHeaderSections(QStringList sections)
{
    for (int i = 0; i < sections.size(); i++)
    {
        QStandardItem *headItem = new QStandardItem(sections.at(i));
        headItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_model->setHorizontalHeaderItem(i, headItem);
    }
}

void DeviceLogTable::setData(const QList<RecordInfo> &infos)
{
    RETURN_IF_TRUE(infos.isEmpty());
    m_model->removeRows(0, m_model->rowCount());

    m_model->setColumnCount(LogTableField::LOG_TABLE_FIELD_LAST);
    m_model->setRowCount(infos.size());

    int row = 0;
    for (int i = 0; i < infos.size(); i++)
    {
        auto recordsInfo = infos.at(i);
        m_model->setData(m_model->index(row, LogTableField::LOG_TABLE_FIELD_NAME), recordsInfo.name);
        m_model->setData(m_model->index(row, LogTableField::LOG_TABLE_FIELD_TYPE), recordsInfo.name);
        m_model->setData(m_model->index(row, LogTableField::LOG_TABLE_FIELD_TIME), recordsInfo.type);
        m_model->setData(m_model->index(row, LogTableField::LOG_TABLE_FIELD_STATUS), recordsInfo.status);
        row++;
    }
}

int DeviceLogTable::getColCount()
{
    return m_model->columnCount();
}

int DeviceLogTable::getRowCount()
{
    return m_model->rowCount();
}

TableFilterModel *DeviceLogTable::getFilterProxy()
{
    return this->m_filterProxy;
}

}  // namespace KS
