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
#include <QAction>
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
#include "common/ssr-marcos-ui.h"
#include "src/ui/common/table/header-button-delegate.h"
#include "src/ui/log/utils.h"
#include "src/ui/log_proxy.h"

namespace KS
{
namespace Log
{
#define ALL_LOG_ROLE 0xFFFF
#define ALL_LOG_TYPE 0xFFFF

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

LogDelegate::LogDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
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

LogFilterModel::LogFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
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

LogModel::LogModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    initGetLogArgs();
    m_logProxy = new LogProxy(SSR_DBUS_NAME,
                              SSR_LOG_DBUS_OBJECT_PATH,
                              QDBusConnection::systemBus(),
                              this);
    connect(m_logProxy, &LogProxy::NewLogWritten, this, [this](uint log_num)
            {
                updateRecord();
            });

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
            return Utils::logTypeEnum2Str(logInfo.type);
        case LogTableField::LOG_TABLE_FIELD_USERNAME:
            return Utils::accountRoleEnum2Str(logInfo.role);
        case LogTableField::LOG_TABLE_FIELD_DATATIME:
            return logInfo.dataTime;
        case LogTableField::LOG_TABLE_FIELD_MESSAGE:
            return logInfo.message;
        case LogTableField::LOG_TABLE_FIELD_RESULT:
            return logInfo.result ? tr("Success") : tr("Failed");
        default:
            break;
        }
        break;
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
            return "";
            //        case LogTableField::LOG_TABLE_FIELD_RESOURCE:
            //            return tr("App resource");
        case LogTableField::LOG_TABLE_FIELD_USERNAME:
            return "";
        case LogTableField::LOG_TABLE_FIELD_DATATIME:
            return tr("Data time");
        case LogTableField::LOG_TABLE_FIELD_MESSAGE:
            return tr("Message");
        case LogTableField::LOG_TABLE_FIELD_RESULT:
            return "";
        default:
            break;
        }
        break;
    }
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
    SCOPE_EXIT(
        {
            endResetModel();
        });
    m_logInfos.clear();
    auto reply = m_logProxy->GetLog(static_cast<int>(m_args.role), m_args.timeStampBegin,
                                    m_args.timeStampEnd, m_args.type,
                                    m_args.result, m_args.searchKey, LOG_PAGE_NUMBER,
                                    m_args.currentPage);
    reply.waitForFinished();
    Utils::deserialize(reply.value(), m_logInfos);
    emit logUpdated(static_cast<int>(getLogNumbers()));
}

void LogModel::initGetLogArgs()
{
    m_args.role = static_cast<AccountRole>(ALL_LOG_ROLE);
    // 一个月前
    m_args.timeStampBegin = QDateTime::currentDateTime().addMonths(-1).toSecsSinceEpoch();
    m_args.timeStampEnd = LONG_LONG_MAX;
    m_args.type = static_cast<LogType>(ALL_LOG_TYPE);
    m_args.result = LOG_RESULT_ALL;
    m_args.currentPage = 1;
}

uint LogModel::getLogNumbers()
{
    auto reply = m_logProxy->GetLogNum(static_cast<int>(m_args.role), m_args.timeStampBegin, m_args.timeStampEnd, m_args.type, m_args.result, m_args.searchKey);
    reply.waitForFinished();
    return reply.value();
}

void LogModel::setRole(uint role)
{
    RETURN_IF_TRUE(m_args.role == role);
    m_args.role = role;
    updateRecord();
}

void LogModel::setTimeStampBegin(qlonglong timeStampBegin)
{
    RETURN_IF_TRUE(m_args.timeStampBegin == timeStampBegin);
    m_args.timeStampBegin = timeStampBegin;
    updateRecord();
}

void LogModel::setTimeStampEnd(qlonglong timeStampEnd)
{
    RETURN_IF_TRUE(m_args.timeStampEnd == timeStampEnd);
    m_args.timeStampEnd = timeStampEnd;
    updateRecord();
}

void LogModel::setLogType(Log::LogType type)
{
    RETURN_IF_TRUE(m_args.type == type);
    m_args.type = type;
    updateRecord();
}

void LogModel::setLogResult(uint result)
{
    RETURN_IF_TRUE(m_args.result == result);
    m_args.result = result;
    updateRecord();
}

void LogModel::setCurrentPage(uint currentPage)
{
    RETURN_IF_TRUE(m_args.currentPage == currentPage);
    m_args.currentPage = currentPage;
    updateRecord();
}

void LogModel::setSearchKey(const QString &text)
{
    m_args.searchKey = text;
    updateRecord();
}

LogTable::LogTable(QWidget *parent)
    : QTableView(parent),
      m_filterProxy(nullptr)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    initTable();
    initTableHeaderButton();
}

void LogTable::search(const QString &text)
{
    m_model->setSearchKey(text);
}

uint LogTable::getLogNumbers()
{
    return m_model->getLogNumbers();
}

void LogTable::setCurrentPage(uint currentPage)
{
    m_model->setCurrentPage(currentPage);
}

void LogTable::setTimeStampBegin(qlonglong timeStampBegin)
{
    m_model->setTimeStampBegin(timeStampBegin);
}

void LogTable::setTimeStampEnd(qlonglong timeStampEnd)
{
    m_model->setTimeStampEnd(timeStampEnd);
}

void LogTable::initTable()
{
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
    m_headerViewProxy->resizeSection(LogTableField::LOG_TABLE_FIELD_LOG_TYPE, 200);
    m_headerViewProxy->resizeSection(LogTableField::LOG_TABLE_FIELD_USERNAME, 100);
    m_headerViewProxy->resizeSection(LogTableField::LOG_TABLE_FIELD_DATATIME, 150);
    m_headerViewProxy->resizeSection(LogTableField::LOG_TABLE_FIELD_MESSAGE, 300);
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
    verticalHeader->hide();

    connect(this, &LogTable::entered, this, &LogTable::mouseEnter);
}

void LogTable::initTableHeaderButton()
{
    initLogTypeButton();
    initRoleButton();
    initResultButton();

    QMap<int, HeaderButtonDelegate *> headerButtons;
    headerButtons.insert(LOG_TABLE_FIELD_LOG_TYPE, m_logTypeButton);
    headerButtons.insert(LOG_TABLE_FIELD_USERNAME, m_roleButton);
    headerButtons.insert(LOG_TABLE_FIELD_RESULT, m_resultButton);
    m_headerViewProxy->setHeaderButtons(headerButtons);
}

void LogTable::initLogTypeButton()
{
    // 日志类型筛选
    m_logTypeButton = new HeaderButtonDelegate(this);
    m_logTypeButton->setButtonText(tr("Log type"));
    auto device = new QAction(tr("Device log"), m_logTypeButton);
    auto toolBox = new QAction(tr("Tool box log"), m_logTypeButton);
    auto baselineReinforcement = new QAction(tr("Baseline reinforcement log"), m_logTypeButton);
    auto trustedProtection = new QAction(tr("Trusted protection log"), m_logTypeButton);
    auto filesProtection = new QAction(tr("Files protection log"), m_logTypeButton);
    auto privateBox = new QAction(tr("Private box log"), m_logTypeButton);
    auto account = new QAction(tr("Account log"), m_logTypeButton);

    m_logTypeButton->addMenuActions(QList<QAction *>() << device << toolBox << baselineReinforcement << trustedProtection << filesProtection << privateBox << account);
    connect(m_logTypeButton, &HeaderButtonDelegate::menuTriggered, this, [this]()
            {
                int type = 0;
                for (auto action : m_logTypeButton->getMenuActions())
                {
                    CONTINUE_IF_TRUE(!action->isChecked());
                    type |= Utils::str2LogTypeEnum(action->text());
                }
                m_model->setLogType(static_cast<LogType>(type));
            });
}

void LogTable::initRoleButton()
{
    // 日志角色筛选
    m_roleButton = new HeaderButtonDelegate(this);
    m_roleButton->setButtonText(tr("User name"));
    auto sysadm = new QAction(tr("Sysadm"), m_roleButton);
    auto secadm = new QAction(tr("Secadm"), m_roleButton);
    auto audadm = new QAction(tr("Audadm"), m_roleButton);

    m_roleButton->addMenuActions(QList<QAction *>() << sysadm << secadm << audadm);
    connect(m_roleButton, &HeaderButtonDelegate::menuTriggered, this, [this]()
            {
                int role = 0;
                for (auto action : m_roleButton->getMenuActions())
                {
                    CONTINUE_IF_TRUE(!action->isChecked());
                    role |= Utils::str2AccountRoleEnum(action->text());
                }
                m_model->setRole(static_cast<AccountRole>(role));
            });
}

void LogTable::initResultButton()
{
    // 日志结果筛选
    m_resultButton = new HeaderButtonDelegate(this);
    m_resultButton->setButtonText(tr("Result"));
    auto success = new QAction(tr("Success"), m_resultButton);
    auto fail = new QAction(tr("Failed"), m_resultButton);

    m_resultButton->addMenuActions(QList<QAction *>() << success << fail);
    connect(m_resultButton, &HeaderButtonDelegate::menuTriggered, this, [this]()
            {
                QMap<QString, bool> roleMap;
                for (auto action : m_resultButton->getMenuActions())
                {
                    roleMap.insert(action->text(), action->isChecked());
                }
                if (roleMap.value(tr("Success")) && roleMap.value(tr("Failed")))
                {
                    // 全选
                    m_model->setLogResult(LOG_RESULT_ALL);
                }
                else
                {
                    // 如果两个都未选中，设置result为3,获取表格为空
                    if (!roleMap.value(tr("Success")) && !roleMap.value(tr("Failed")))
                    {
                        m_model->setLogResult(3);
                        return;
                    }
                    // 仅一个选中
                    m_model->setLogResult(roleMap.value(tr("Success")) ? LOG_RESULT_TRUE : LOG_RESULT_FALSE);
                }
            });
}

void LogTable::mouseEnter(const QModelIndex &index)
{
    RETURN_IF_TRUE(index.column() != LogTableField::LOG_TABLE_FIELD_MESSAGE);
    auto mod = selectionModel()->model()->data(index);
    QToolTip::showText(QCursor::pos(), mod.toString(), this, rect(), 5000);
}
}  // namespace Log
}  // namespace KS
