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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "device-list-table.h"
#include <QAction>
#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QToolTip>
#include "include/ssr-marcos.h"
#include "src/ui/common/table/header-button-delegate.h"
#include "src/ui/common/table/table-header-proxy.h"
#include "src/ui/dm/table-filter-model.h"
#include "src/ui/dm/utils.h"

namespace KS
{
namespace DM
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
    m_deviceManagerProxy = new DeviceManagerProxy(SSR_DBUS_NAME,
                                                  SSR_DEVICE_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);
    initTable();
    initTableHeaderButton();
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

        auto type = Utils::deviceTypeEnum2Str(deviceInfo.type);
        auto interface = Utils::interfaceTypeEnum2Str(deviceInfo.interface);
        auto state = Utils::deviceStateEnum2Str(deviceInfo.state);

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

    auto index = m_filterProxy->index(row, ListTableField::LIST_TABLE_FIELD_STATUS);
    return Utils::deviceStateStr2Enum(m_filterProxy->data(index).toString());
}

QString DeviceListTable::getType(int row)
{
    if (row >= m_devicesInfo.size())
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QString();
    }

    auto index = m_filterProxy->index(row, ListTableField::LIST_TABLE_FIELD_TYPE);
    return m_filterProxy->data(index).toString();
}

QString DeviceListTable::getID(int row)
{
    if (row >= m_devicesInfo.size())
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QString();
    }

    auto index = m_filterProxy->index(row, ListTableField::LIST_TABLE_FIELD_ID);
    return m_filterProxy->data(index).toString();
}

QString DeviceListTable::getName(int row)
{
    if (row >= m_devicesInfo.size())
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QString();
    }

    auto index = m_filterProxy->index(row, ListTableField::LIST_TABLE_FIELD_NAME);
    return m_filterProxy->data(index).toString();
}

int DeviceListTable::getPermission(int row)
{
    if (row >= m_devicesInfo.size())
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return -1;
    }

    auto index = m_filterProxy->index(row, ListTableField::LIST_TABLE_FIELD_PERMISSION);
    return m_filterProxy->data(index).toInt();
}

int DeviceListTable::getColCount()
{
    return m_model->columnCount();
}

int DeviceListTable::getRowCount()
{
    return m_model->rowCount();
}

void DeviceListTable::setSearchText(const QString &text)
{
    m_searchText = text;
    m_filterProxy->setSearchText(m_searchText);
    filterFixedString();
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
    setShowGrid(false);

    // 设置Model
    m_model = new QStandardItemModel(this);
    m_filterProxy = new TableFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);

    // 设置代理
    setItemDelegate(new DeviceListDelegate(this));

    // 设置水平行表头
    m_headerViewProxy = new TableHeaderProxy(this);
    m_headerViewProxy->hideCheckBox(true);
    m_headerViewProxy->setStretchLastSection(true);
    m_headerViewProxy->setSectionsMovable(false);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_headerViewProxy->setFixedHeight(24);
    setHorizontalHeader(m_headerViewProxy);

    setHeaderSections(QStringList() << tr("Number")
                                    << tr("Device Name")
                                    << ""
                                    << tr("Device Id")
                                    << tr("Interface")
                                    << ""
                                    << tr("Permission"));
    m_headerViewProxy->resizeSection(ListTableField::LIST_TABLE_FIELD_NUMBER, 60);
    m_headerViewProxy->resizeSection(ListTableField::LIST_TABLE_FIELD_NAME, 180);
    m_headerViewProxy->resizeSection(ListTableField::LIST_TABLE_FIELD_STATUS, 100);
    m_headerViewProxy->resizeSection(ListTableField::LIST_TABLE_FIELD_INTERFACE, 80);
    m_headerViewProxy->resizeSection(ListTableField::LIST_TABLE_FIELD_TYPE, 100);
    m_headerViewProxy->resizeSection(ListTableField::LIST_TABLE_FIELD_ID, 80);

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

void DeviceListTable::initTableHeaderButton()
{
    // 文件类型筛选
    m_deviceTypeButton = new HeaderButtonDelegate(this);
    m_deviceTypeButton->setButtonText(tr("Device Type"));

    auto storage = new QAction(tr("Storage"), m_deviceTypeButton);
    auto cd = new QAction(tr("CD"), m_deviceTypeButton);
    auto mouse = new QAction(tr("Mouse"), m_deviceTypeButton);
    auto keyboard = new QAction(tr("Keyboard"), m_deviceTypeButton);
    auto network = new QAction(tr("Network card"), m_deviceTypeButton);
    auto wireless = new QAction(tr("Wireless network card"), m_deviceTypeButton);
    auto video = new QAction(tr("Video"), m_deviceTypeButton);
    auto audio = new QAction(tr("Audio"), m_deviceTypeButton);
    auto printer = new QAction(tr("Printer"), m_deviceTypeButton);
    auto hub = new QAction(tr("Hub"), m_deviceTypeButton);
    auto communications = new QAction(tr("Communications"), m_deviceTypeButton);
    auto bluetooth = new QAction(tr("Bluetooth"), m_deviceTypeButton);
    auto unknown = new QAction(tr("Unknown"), m_deviceTypeButton);
    m_deviceTypeKeys << tr("Storage") << tr("CD") << tr("Mouse") << tr("Keyboard") << tr("Network card") << tr("Wireless network card") << tr("Video")
                     << tr("Audio") << tr("Printer") << tr("Hub") << tr("Communications")
                     << tr("Bluetooth") << tr("Unknown");
    m_filterMap.insert("deviceTypeButton", m_deviceTypeKeys);
    m_deviceTypeButton->addMenuActions(QList<QAction *>() << storage << cd << mouse << keyboard << network << wireless << video << audio << printer << hub << communications << bluetooth << unknown);
    connect(m_deviceTypeButton, &HeaderButtonDelegate::menuTriggered, this, [this]() {
        for (auto action : m_deviceTypeButton->getMenuActions())
        {
            if (action->isChecked())
            {
                m_deviceTypeKeys << action->text();
            }
            else
            {
                m_deviceTypeKeys.removeAll(action->text());
            }
            // 去重
            m_deviceTypeKeys = QSet<QString>::fromList(m_deviceTypeKeys).toList();
            m_filterMap.insert("deviceTypeButton", m_deviceTypeKeys);
        }
        filterFixedString();
    });
    // 状态筛选
    m_statusButton = new HeaderButtonDelegate(this);
    m_statusButton->setButtonText(tr("Status"));

    auto enable = new QAction(ENABLE, m_statusButton);
    auto disable = new QAction(DISABLE, m_statusButton);
    auto unauthoried = new QAction(UNAUTHORIED, m_statusButton);
    m_statusKeys << ENABLE << DISABLE << UNAUTHORIED;
    m_filterMap.insert("statusButton", m_statusKeys);
    m_statusButton->addMenuActions(QList<QAction *>() << enable << disable << unauthoried);
    connect(m_statusButton, &HeaderButtonDelegate::menuTriggered, this, [this]() {
        for (auto action : m_statusButton->getMenuActions())
        {
            if (action->isChecked())
            {
                m_statusKeys << action->text();
            }
            else
            {
                m_statusKeys.removeAll(action->text());
            }
            // 去重
            m_statusKeys = QSet<QString>::fromList(m_statusKeys).toList();
            m_filterMap.insert("statusButton", m_statusKeys);
        }
        filterFixedString();
    });
    QMap<int, HeaderButtonDelegate *> headerButtons;
    headerButtons.insert(LIST_TABLE_FIELD_TYPE, m_deviceTypeButton);
    headerButtons.insert(LIST_TABLE_FIELD_STATUS, m_statusButton);
    m_headerViewProxy->setHeaderButtons(headerButtons);
    filterFixedString();
}

void DeviceListTable::filterFixedString()
{
    QStringList patternList = {};
    for (auto value : m_filterMap.values())
    {
        CONTINUE_IF_TRUE(value.isEmpty());
        QStringList keys;
        for (auto key : value)
        {
            CONTINUE_IF_TRUE(key.isEmpty());
            keys << key;
        }
        patternList << keys.join("|");
    }
    QString pattern = "(" + patternList.join(").*(") + ")";
    KLOG_DEBUG() << "The search text is change to " << pattern;
    m_filterProxy->setFilterRegExp(pattern);
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

    //FIXME:计算字符像素宽度不准确
    auto itemText = m_filterProxy->data(index).toString();
    RETURN_IF_TRUE(itemText.isEmpty());

    QFontMetrics fm(fontMetrics());
    auto textRect = fm.boundingRect(itemText);
    const int textMargin = this->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    //文字矩形宽要加上单元格左边padding的10px
    auto textWidthInPxs = textRect.width() + 10 + textMargin;

    if (textWidthInPxs > columnWidth(index.column()))
    {
        QPoint point = QCursor::pos();
        QRect rect = QRect(point.x(), point.y(), 30, 10);
        QToolTip::showText(point, itemText, this, rect);
    }
    else
    {
        QToolTip::hideText();
    }
}

#define GET_JSON_BOOL_VALUE(obj, key) ((obj).value(key).isBool() ? (obj).value(key).toBool() : false)

#define SET_DEVICE_PERMISSION(obj, key, deviceInfo, permissionType) \
    if (GET_JSON_BOOL_VALUE(obj, key))                              \
        deviceInfo.permission |= permissionType;

void DeviceListTable::update()
{
    m_devicesInfo.clear();
    auto reply = m_deviceManagerProxy->GetDevicesByInterface(InterfaceType::INTERFACE_TYPE_USB);
    reply.waitForFinished();
    auto devicesJson = reply.value();
    KLOG_DEBUG() << "The reply of dbus method GetDevicesByInterface:" << devicesJson;

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
        auto usbId = data.value(SSR_DEVICE_JK_ID).toString();
        // 供应商 ID 1d6b 代表是 linux 内核提供的虚拟 usb 设备，所以不在我们管控范围内，故不予显示。
        if (usbId.startsWith("1d6b", Qt::CaseInsensitive) ||
            // 供应商 ID 0bda 代表是 Realtek 公司提供的 usb 设备， 5411 和 0411 都是 usb 集线器的设备号，不予显示。
            usbId.startsWith("0bda:5411", Qt::CaseInsensitive) ||
            usbId.startsWith("0bda:0411", Qt::CaseInsensitive))
        {
            continue;
        }

        auto deviceInfo = DeviceInfo{.number = count,
                                     .name = data.value(SSR_DEVICE_JK_NAME).toString(),
                                     .type = (DeviceType)data.value(SSR_DEVICE_JK_TYPE).toInt(),
                                     .id = usbId,
                                     .interface = (InterfaceType)data.value(SSR_DEVICE_JK_INTERFACE_TYPE).toInt(),
                                     .state = (DeviceState)data.value(SSR_DEVICE_JK_STATE).toInt(),
                                     .permission = 0};

        SET_DEVICE_PERMISSION(data, SSR_DEVICE_JK_READ, deviceInfo, PermissionType::PERMISSION_TYPE_READ);
        SET_DEVICE_PERMISSION(data, SSR_DEVICE_JK_WRITE, deviceInfo, PermissionType::PERMISSION_TYPE_WRITE);
        SET_DEVICE_PERMISSION(data, SSR_DEVICE_JK_EXECUTE, deviceInfo, PermissionType::PERMISSION_TYPE_EXEC);

        m_devicesInfo.push_back(deviceInfo);
        count++;
    }
    setData(m_devicesInfo);
}

}  // namespace DM
}  // namespace KS
