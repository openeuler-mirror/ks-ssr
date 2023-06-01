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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "device-log-table.h"
#include <QApplication>
#include <QDateTime>
#include <QFont>
#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include "include/ksc-marcos.h"
#include "src/ui/device/device-utils.h"
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

    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);

    //绘制的文字向右偏移10px，与整体表格风格统一
    auto textRect = option.rect.adjusted(10, 0, 0, 0);

    //绘制状态列:根据状态显示字体颜色
    if (index.column() == LogTableField::LOG_TABLE_FIELD_STATUS)
    {
        //TODO: 由于翻译成中文后使用map方式获取不到颜色值，后面要优化逻辑
        auto state = index.data().toString();
        QColor color;
        if (state == SUCCESSFUL)
        {
            color.setNamedColor("#00a2ff");
        }
        else
        {
            color.setNamedColor("#d30000");
        }
        viewOption.palette.setColor(QPalette::Text, color);

        QApplication::style()->drawItemText(painter,
                                            textRect,
                                            Qt::AlignLeft | Qt::AlignVCenter,
                                            viewOption.palette,
                                            true,
                                            index.data().toString(),
                                            QPalette::Text);
    }
    else
    {
        this->QStyledItemDelegate::paint(painter, option, index);
    }
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
    connect(m_deviceManagerProxy, &DeviceManagerProxy::DeviceChanged, this, &DeviceLogTable::update);

    initTable();
}

void DeviceLogTable::initTable()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionMode(QAbstractItemView::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);

    // 设置Model
    m_model = new QStandardItemModel(this);
    m_filterProxy = new TableFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);
    setShowGrid(false);

    // 设置代理
    setItemDelegate(new DeviceLogDelegate(this));

    // 设置水平行表头
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionsMovable(false);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader()->setFixedHeight(24);
    horizontalHeader()->setDefaultAlignment(Qt::AlignVCenter);
    setHeaderSections(QStringList() << tr("Device Name")
                                    << tr("Device Type")
                                    << tr("Time")
                                    << tr("Device Status"));
    horizontalHeader()->resizeSection(LogTableField::LOG_TABLE_FIELD_NAME, 200);
    horizontalHeader()->resizeSection(LogTableField::LOG_TABLE_FIELD_TIME, 200);
    horizontalHeader()->resizeSection(LogTableField::LOG_TABLE_FIELD_TYPE, 200);

    // 设置垂直列表头
    verticalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(38);
}

void DeviceLogTable::update()
{
    m_recordsInfo.clear();
    auto reply = m_deviceManagerProxy->GetRecords();
    reply.waitForFinished();
    auto recordJson = reply.value();
    KLOG_DEBUG() << "The reply of dbus method GetRecords:" << recordJson;

    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(recordJson.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files information failed: " << jsonError.errorString();
        return;
    }

    auto jsonDataArray = jsonDoc.array();
    // 倒序排序
    auto jsonData = jsonDataArray.end();
    while (jsonData != jsonDataArray.begin())
    {
        jsonData--;
        auto data = jsonData->toObject();
        auto recordInfo = RecordInfo{.name = data.value(KSC_DCR_JK_NAME).toString(),
                                     .type = (DeviceType)data.value(KSC_DCR_JK_TYPE).toInt(),
                                     .time = data.value(KSC_DCR_JK_TIME).toVariant().toUInt(),
                                     .state = (DeviceConnectState)data.value(KSC_DCR_JK_STATE).toInt()};
        m_recordsInfo.push_back(recordInfo);
    }
    setData(m_recordsInfo);
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

        auto type = DeviceUtils::deviceTypeEnum2Str(recordsInfo.type);
        auto state = DeviceUtils::deviceConnectStateEnum2Str(recordsInfo.state);
        auto time = QDateTime::fromSecsSinceEpoch(recordsInfo.time);

        m_model->setData(m_model->index(row, LogTableField::LOG_TABLE_FIELD_NAME), recordsInfo.name);
        m_model->setData(m_model->index(row, LogTableField::LOG_TABLE_FIELD_TYPE), type);
        m_model->setData(m_model->index(row, LogTableField::LOG_TABLE_FIELD_TIME), time);
        m_model->setData(m_model->index(row, LogTableField::LOG_TABLE_FIELD_STATUS), state);
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
    return m_filterProxy;
}

}  // namespace KS
