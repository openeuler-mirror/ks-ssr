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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#include "src/ui/log/table.h"
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
#include "src/ui/log_proxy.h"
#include "ssr-i.h"
#include "ssr-marcos.h"

namespace KS
{
namespace Log
{
enum LogTableField
{
    LOG_TABLE_FIELD_NUMBER,
    LOG_TABLE_FIELD_LOG_TYPE,
    //    LOG_TABLE_FIELD_RESOURCE,
    LOG_TABLE_FIELD_USERNAME,
    LOG_TABLE_FIELD_DATATIME,
    LOG_TABLE_FIELD_MESSAGE,
    LOG_TABLE_FIELD_RESULT,
    LOG_TABLE_FIELD_LAST
};

// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

LogDelegate::LogDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

LogDelegate::~LogDelegate()
{
    KLOG_DEBUG() << "The LogDelegate is deleted.";
}

void LogDelegate::paint(QPainter *painter,
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

    QStyledItemDelegate::paint(painter, option, index);
}

LogFilterModel::LogFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool LogFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    for (auto i = 0; i < LOG_TABLE_FIELD_LAST; ++i)
    {
        auto index = sourceModel()->index(sourceRow, i, sourceParent);
        auto text = sourceModel()->data(index).toString();
        RETURN_VAL_IF_TRUE(text.contains(filterRegExp()), true);
    }

    return false;
}

LogModel::LogModel(QObject *parent) : QAbstractTableModel(parent)
{
    m_logProxy = new LogProxy(SSR_DBUS_NAME,
                              SSR_KSS_INIT_DBUS_OBJECT_PATH,
                              QDBusConnection::systemBus(),
                              this);
    // TODO 连接后台信号DetectHazard, 使用NotificationWrapper弹窗
    //    connect(m_logProxy, &LogProxy::连接后台信号DetectHazard, this, &LogModel::updateRecord);

    updateRecord();
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    return m_logInfos.size();
}

int LogModel::columnCount(const QModelIndex &parent) const
{
    return LogTableField::LOG_TABLE_FIELD_LAST;
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

    if (index.row() >= m_logInfos.size() || index.column() >= LogTableField::LOG_TABLE_FIELD_LAST)
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QVariant();
    }

    auto logInfo = m_logInfos[index.row()];
    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (index.column())
        {
        case LogTableField::LOG_TABLE_FIELD_NUMBER:
            return index.row() + 1;
        case LogTableField::LOG_TABLE_FIELD_LOG_TYPE:
            // TODO type2str
            return logInfo.type;
            //        case LogTableField::LOG_TABLE_FIELD_RESOURCE:
            //            return logInfo.resource;
        case LogTableField::LOG_TABLE_FIELD_USERNAME:
            return logInfo.userName;
        case LogTableField::LOG_TABLE_FIELD_DATATIME:
            return logInfo.dataTime;
        case LogTableField::LOG_TABLE_FIELD_MESSAGE:
            return logInfo.message;
        case LogTableField::LOG_TABLE_FIELD_RESULT:
            return logInfo.result;
        default:
            break;
        }
    }
    default:
        break;
    }

    return QVariant();
}

QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        case LogTableField::LOG_TABLE_FIELD_NUMBER:
            return tr("Number");
        case LogTableField::LOG_TABLE_FIELD_LOG_TYPE:
            return tr("Log type");
            //        case LogTableField::LOG_TABLE_FIELD_RESOURCE:
            //            return tr("App resource");
        case LogTableField::LOG_TABLE_FIELD_USERNAME:
            return tr("User name");
        case LogTableField::LOG_TABLE_FIELD_DATATIME:
            return tr("Data time");
        case LogTableField::LOG_TABLE_FIELD_MESSAGE:
            return tr("Message");
        case LogTableField::LOG_TABLE_FIELD_RESULT:
            return tr("Result");
        default:
            break;
        }
    }
    break;
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags LogModel::flags(const QModelIndex &index) const
{
    return Qt::ItemFlag::NoItemFlags;
}

void LogModel::updateRecord()
{
    beginResetModel();
    SCOPE_EXIT({
        endResetModel();
    });

    m_logInfos.clear();
    // TODO 从后台取数据，存入m_logInfos
    // auto reply = m_logProxy->GetProtectedFiles();

    emit logUpdated(m_logInfos.size());
}

QList<LogInfo> LogModel::getLogInfos()
{
    return m_logInfos;
}

LogTable::LogTable(QWidget *parent) : QTableView(parent),
                                      m_filterProxy(nullptr)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置Model
    m_model = new LogModel(this);
    m_headerViewProxy = new TableHeaderProxy(this);
    m_headerViewProxy->hideCheckBox(true);
    setHorizontalHeader(m_headerViewProxy);
    setMouseTracking(true);
    connect(m_model, &LogModel::logUpdated, this, &LogTable::logUpdated);

    m_filterProxy = new LogFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);
    setShowGrid(false);

    // 设置Delegate
    setItemDelegate(new LogDelegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(LogTableField::LOG_TABLE_FIELD_NUMBER, 50);
    m_headerViewProxy->resizeSection(LogTableField::LOG_TABLE_FIELD_LOG_TYPE, 100);
    m_headerViewProxy->resizeSection(LogTableField::LOG_TABLE_FIELD_USERNAME, 100);
    m_headerViewProxy->resizeSection(LogTableField::LOG_TABLE_FIELD_DATATIME, 200);
    m_headerViewProxy->resizeSection(LogTableField::LOG_TABLE_FIELD_MESSAGE, 200);
    m_headerViewProxy->resizeSection(LogTableField::LOG_TABLE_FIELD_RESULT, 100);
    m_headerViewProxy->setStretchLastSection(true);
    m_headerViewProxy->setSectionsMovable(false);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignLeft);
    m_headerViewProxy->setFixedHeight(24);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignVCenter);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
}

void LogTable::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;
    // TODO 传数据给后台，后台进行关键字搜索
    m_filterProxy->setFilterFixedString(text);
}

QList<LogInfo> LogTable::getLogInfos()
{
    return m_model->getLogInfos();
}

}  // namespace Log
}  // namespace KS
