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

#include "src/ui/tool-box/access-control/access-control-table.h"
#include <stdio.h>
#include <QApplication>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStandardItemModel>
#include <QStyle>
#include <QToolTip>
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/common/table/table-header-proxy.h"
#include "src/ui/toolbox_dbus_proxy.h"
#include "ssr-i.h"

namespace KS
{
namespace ToolBox
{
enum AccessControlTableField
{
    ACCESS_CONTROL_TABLE_FIELD_USER_NAME,
    ACCESS_CONTROL_TABLE_FIELD_USER_FUNCTION,
    ACCESS_CONTROL_TABLE_FIELD_LAST
};

// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

AccessControlDelegate::AccessControlDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

AccessControlDelegate::~AccessControlDelegate()
{
    KLOG_DEBUG() << "The AccessControlDelegate is deleted.";
}

void AccessControlDelegate::paint(QPainter *painter,
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

AccessControlModel::AccessControlModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int AccessControlModel::rowCount(const QModelIndex &parent) const
{
    return m_infos.size();
}

int AccessControlModel::columnCount(const QModelIndex &parent) const
{
    return AccessControlTableField::ACCESS_CONTROL_TABLE_FIELD_LAST;
}

QVariant AccessControlModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

    if (index.row() >= m_infos.size() || index.column() >= AccessControlTableField::ACCESS_CONTROL_TABLE_FIELD_LAST)
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QVariant();
    }

    auto info = m_infos[index.row()];
    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (index.column())
        {
        case AccessControlTableField::ACCESS_CONTROL_TABLE_FIELD_USER_NAME:
            return info.userName;
        case AccessControlTableField::ACCESS_CONTROL_TABLE_FIELD_USER_FUNCTION:
            return info.userFunction;
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

QVariant AccessControlModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        case AccessControlTableField::ACCESS_CONTROL_TABLE_FIELD_USER_NAME:
            return tr("User name");
        case AccessControlTableField::ACCESS_CONTROL_TABLE_FIELD_USER_FUNCTION:
            return tr("User function");
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

Qt::ItemFlags AccessControlModel::flags(const QModelIndex &index) const
{
    return Qt::ItemFlag::NoItemFlags;
}

void AccessControlModel::showInfos(bool isShow)
{
    beginResetModel();
    SCOPE_EXIT({
        endResetModel();
    });
    m_infos.clear();
    RETURN_IF_TRUE(!isShow);

    // 这个表格是写死的，不可修改，暂时手动写入数据
    auto sysadmInfo = AccessControlInfo{
        .userName = tr("sysadm(system admin)"),
        .userFunction = tr("Use, Network, Soft, Password, Device, Time, Drive, Resource, Log manager")};
    auto secadmInfo = AccessControlInfo{
        .userName = tr("secadm(secure admin)"),
        .userFunction = tr("User role, Access policy, CAP authorzation modification, Role attribute, files secure manager")};
    auto audadmInfo = AccessControlInfo{
        .userName = tr("audadm(audit admin)"),
        .userFunction = tr("Secure audit, Audit policy manager, Audit log manager")};

    m_infos << sysadmInfo << secadmInfo << audadmInfo;
}

AccessControlTable::AccessControlTable(QWidget *parent)
    : QTableView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_dbusProxy = new ToolBoxDbusProxy(SSR_DBUS_NAME,
                                       SSR_TOOL_BOX_DBUS_OBJECT_PATH,
                                       QDBusConnection::systemBus(),
                                       this);
    initTable();
}

bool AccessControlTable::openSelinux(bool isOpen)
{
    auto reply = m_dbusProxy->SetAccessControlStatus(isOpen);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    RETURN_VAL_IF_TRUE(reply.isError(), false);
    m_model->showInfos(isOpen);
    isOpen ? m_headerViewProxy->show() : m_headerViewProxy->hide();
    return true;
}

bool ToolBox::AccessControlTable::getSelinuxStatus()
{
    auto reply = m_dbusProxy->GetAccessStatus();
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    RETURN_VAL_IF_TRUE(reply.isError(), false);

    return reply.value();
}

void KS::ToolBox::AccessControlTable::initTable()
{
    setSelectionMode(QAbstractItemView::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFocusPolicy(Qt::NoFocus);
    setCornerButtonEnabled(false);
    // 设置Model
    m_model = new AccessControlModel(this);
    m_headerViewProxy = new TableHeaderProxy(this);
    m_headerViewProxy->hideCheckBox(true);
    setHorizontalHeader(m_headerViewProxy);
    setMouseTracking(true);
    setModel(m_model);
    setShowGrid(false);
    // 设置Delegate
    setItemDelegate(new AccessControlDelegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(AccessControlTableField::ACCESS_CONTROL_TABLE_FIELD_USER_NAME, 100);
    m_headerViewProxy->resizeSection(AccessControlTableField::ACCESS_CONTROL_TABLE_FIELD_USER_FUNCTION, 500);
    m_headerViewProxy->setStretchLastSection(true);
    m_headerViewProxy->setSectionsMovable(false);
    m_headerViewProxy->setFixedHeight(24);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignLeft);
    m_headerViewProxy->setSectionsClickable(false);
    m_headerViewProxy->setSectionResizeMode(QHeaderView::Fixed);
    m_headerViewProxy->resizeSection(-1, 0);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
    verticalHeader->hide();
}

}  // namespace ToolBox
}  // namespace KS
