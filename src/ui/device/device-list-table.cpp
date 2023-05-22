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

#include "device-list-table.h"
#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QToolTip>
#include "include/ksc-marcos.h"
#include "src/ui/device/device-utils.h"
#include "src/ui/device/table-filter-model.h"

namespace KS
{
// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

DeviceListDelegate::DeviceListDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

DeviceListDelegate::~DeviceListDelegate()
{
}

void DeviceListDelegate ::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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

    //绘制编辑列:字体样式,字体颜色
    if (index.column() == LIST_TABLE_FIELD_PERMISSION)
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
    //绘制状态列:根据状态显示字体颜色
    else if (index.column() == LIST_TABLE_FIELD_STATUS)
    {
        //TODO: 由于翻译成中文后使用map方式获取不到颜色值，后面要优化逻辑
        auto state = index.data(Qt::EditRole).toString();
        QColor color;
        if (state == ENABLE)
        {
            color.setNamedColor("#00a2ff");
        }
        else if (state == DISABLE)
        {
            color.setNamedColor("#d30000");
        }
        else
        {
            color.setNamedColor("#919191");
        }
        viewOption.palette.setColor(QPalette::Text, color);

        QFont font;
        font.setUnderline(false);
        painter->setFont(font);
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

DeviceListTable::DeviceListTable(QWidget *parent) : QTableView(parent),
                                                    m_filterProxy(nullptr),
                                                    m_model(nullptr),
                                                    m_deviceManagerProxy(nullptr)
{
    m_deviceManagerProxy = new DeviceManagerProxy(KSC_DBUS_NAME,
                                                  KSC_DEVICE_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);
    connect(m_deviceManagerProxy, &DeviceManagerProxy::DeviceChanged, this, &DeviceListTable::update);
    initTable();
}

void DeviceListTable::setData(const QList<DeviceInfo> &infos)
{
    RETURN_IF_TRUE(infos.isEmpty());
    m_model->removeRows(0, m_model->rowCount());

    m_model->setColumnCount(ListTableField::LIST_TABLE_FIELD_LAST);
    m_model->setRowCount(infos.size());

    int row = 0;
    for (int i = 0; i < infos.size(); i++)
    {
        auto deviceInfo = infos.at(i);

        auto type = DeviceUtils::deviceTypeEnum2Str(deviceInfo.type);
        auto interface = DeviceUtils::interfaceTypeEnum2Str(deviceInfo.interface);
        auto state = DeviceUtils::deviceStateEnum2Str(deviceInfo.state);

        m_model->setData(m_model->index(row, ListTableField::LIST_TABLE_FIELD_NUMBER), deviceInfo.number);
        m_model->setData(m_model->index(row, ListTableField::LIST_TABLE_FIELD_NAME), deviceInfo.name);
        m_model->setData(m_model->index(row, ListTableField::LIST_TABLE_FIELD_TYPE), type);
        m_model->setData(m_model->index(row, ListTableField::LIST_TABLE_FIELD_ID), deviceInfo.id);
        m_model->setData(m_model->index(row, ListTableField::LIST_TABLE_FIELD_INTERFACE), interface);
        m_model->setData(m_model->index(row, ListTableField::LIST_TABLE_FIELD_STATUS), state);
        m_model->setData(m_model->index(row, ListTableField::LIST_TABLE_FIELD_PERMISSION), deviceInfo.permission);
        row++;
    }
}

DeviceState DeviceListTable::getState(int row)
{
    if (row >= m_devicesInfo.size())
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return DeviceState::DEVICE_STATE_UNAUTHORIED;
    }

    auto item = m_model->item(row, ListTableField::LIST_TABLE_FIELD_STATUS);
    return DeviceUtils::deviceStateStr2Enum(item->text());
}

QString DeviceListTable::getID(int row)
{
    if (row >= m_devicesInfo.size())
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QString();
    }

    auto item = m_model->item(row, ListTableField::LIST_TABLE_FIELD_ID);
    return item->text();
}

QString DeviceListTable::getName(int row)
{
    if (row >= m_devicesInfo.size())
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QString();
    }

    auto item = m_model->item(row, ListTableField::LIST_TABLE_FIELD_NAME);
    return item->text();
}

int DeviceListTable::getPermission(int row)
{
    if (row >= m_devicesInfo.size())
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return -1;
    }

    auto item = m_model->item(row, ListTableField::LIST_TABLE_FIELD_PERMISSION);
    return item->text().toInt();
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
    return m_filterProxy;
}

void DeviceListTable::leaveEvent(QEvent *event)
{
    auto mouseEvent = static_cast<QMouseEvent *>(event);
    //处理鼠标移出表格事件，将鼠标变为箭头
    if (mouseEvent->type() == QEvent::Leave)
    {
        setCursor(Qt::ArrowCursor);
    }
}

void DeviceListTable::initTable()
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
    setItemDelegate(new DeviceListDelegate(this));

    // 设置水平行表头
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionsMovable(false);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader()->setFixedHeight(24);
    horizontalHeader()->setDefaultAlignment(Qt::AlignVCenter);

    setHeaderSections(QStringList() << tr("Number")
                                    << tr("Device Name")
                                    << tr("Device Type")
                                    << tr("Device Id")
                                    << tr("Interface")
                                    << tr("Status")
                                    << tr("Permission"));
    horizontalHeader()->resizeSection(ListTableField::LIST_TABLE_FIELD_NUMBER, 60);
    horizontalHeader()->resizeSection(ListTableField::LIST_TABLE_FIELD_NAME, 150);
    horizontalHeader()->resizeSection(ListTableField::LIST_TABLE_FIELD_STATUS, 100);
    horizontalHeader()->resizeSection(ListTableField::LIST_TABLE_FIELD_INTERFACE, 80);
    horizontalHeader()->resizeSection(ListTableField::LIST_TABLE_FIELD_TYPE, 120);
    horizontalHeader()->resizeSection(ListTableField::LIST_TABLE_FIELD_ID, 100);

    // 设置垂直列表头
    verticalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(38);

    connect(this, &DeviceListTable::entered, this, &DeviceListTable::updateCusor);
    connect(this, &DeviceListTable::entered, this, &DeviceListTable::showDetails);
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

void DeviceListTable::updateCusor(const QModelIndex &index)
{
    RETURN_IF_TRUE(!index.isValid());
    RETURN_IF_TRUE(index.column() > m_model->columnCount() || index.row() > m_model->rowCount());

    /*监听QTableView鼠标entered信号，当鼠标置于权限列时，光标变为手形，
    但是当鼠标直接从编辑列移出表格时，鼠标还是手形，需要处理鼠标移出表格事件，将鼠标变为箭头*/
    if (index.column() == ListTableField::LIST_TABLE_FIELD_PERMISSION)
    {
        setCursor(Qt::PointingHandCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
}

void DeviceListTable::showDetails(const QModelIndex &index)
{
    RETURN_IF_TRUE(!index.isValid());
    RETURN_IF_TRUE(index.column() > m_model->columnCount() || index.row() > m_model->rowCount());
    RETURN_IF_TRUE(index.column() == ListTableField::LIST_TABLE_FIELD_STATUS ||
                   index.column() == ListTableField::LIST_TABLE_FIELD_PERMISSION);

    auto item = m_model->item(index.row(), index.column());
    if (item)
    {
        QFontMetrics fm(fontMetrics());
        auto textWidthInPxs = fm.horizontalAdvance(item->text(), item->text().length());
        if (textWidthInPxs > columnWidth(index.column()))
        {
            QPoint point = QCursor::pos();
            QRect rect = QRect(point.x(), point.y(), 30, 10);
            QToolTip::showText(point, item->text(), this, rect);
        }
        else
        {
            QToolTip::hideText();
        }
    }
}

#define GET_JSON_BOOL_VALUE(obj, key) ((obj).value(key).isBool() ? (obj).value(key).toBool() : false)

#define SET_DEVICE_PERMISSION(obj, key, deviceInfo, permissionType) \
    if (GET_JSON_BOOL_VALUE(obj, key))                              \
        deviceInfo.permission |= permissionType;

void DeviceListTable::update()
{
    m_devicesInfo.clear();
    auto reply = m_deviceManagerProxy->GetDevices();
    reply.waitForFinished();
    auto devicesJson = reply.value();
    KLOG_DEBUG() << "The reply of dbus method GetDevices:" << devicesJson;

    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(devicesJson.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files information failed: " << jsonError.errorString();
        return;
    }

    int count = 1;
    auto jsonDataArray = jsonDoc.array();
    for (auto jsonData : jsonDataArray)
    {
        auto data = jsonData.toObject();

        auto deviceInfo = DeviceInfo{.number = count,
                                     .name = data.value(KSC_DEVICE_JK_NAME).toString(),
                                     .type = (DeviceType)data.value(KSC_DEVICE_JK_TYPE).toInt(),
                                     .id = data.value(KSC_DEVICE_JK_ID).toString(),
                                     .interface = (InterfaceType)data.value(KSC_DEVICE_JK_INTERFACE_TYPE).toInt(),
                                     .state = (DeviceState)data.value(KSC_DEVICE_JK_STATE).toInt(),
                                     .permission = 0};

        SET_DEVICE_PERMISSION(data, KSC_DEVICE_JK_READ, deviceInfo, PermissionType::PERMISSION_TYPE_READ);
        SET_DEVICE_PERMISSION(data, KSC_DEVICE_JK_WRITE, deviceInfo, PermissionType::PERMISSION_TYPE_WRITE);
        SET_DEVICE_PERMISSION(data, KSC_DEVICE_JK_EXECUTE, deviceInfo, PermissionType::PERMISSION_TYPE_EXEC);

        m_devicesInfo.push_back(deviceInfo);
        count++;
    }
    setData(m_devicesInfo);
}

}  // namespace KS
