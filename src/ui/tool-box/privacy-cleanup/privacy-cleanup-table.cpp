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

#include "src/ui/tool-box/privacy-cleanup/privacy-cleanup-table.h"
#include <stdio.h>
#include <QApplication>
#include <QCheckBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTableView>
#include <QToolTip>
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/common/table/table-header-proxy.h"
#include "src/ui/toolbox_dbus_proxy.h"
#include "ssr-i.h"

namespace KS
{
namespace ToolBox
{
enum PrivacyCleanupTableField
{
    PRIVACY_CLEANUP_TABLE_FIELD_CHECKBOX,
    PRIVACY_CLEANUP_TABLE_FIELD_NUMBER,
    PRIVACY_CLEANUP_TABLE_FIELD_USER_NAME,
    PRIVACY_CLEANUP_TABLE_FIELD_USER_TYPE,
    PRIVACY_CLEANUP_TABLE_FIELD_LAST
};

// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

// 表格json信息key
#define USER_NAME_JSON_KEY "name"
#define USER_TYPE_JSON_KEY "type"

PrivacyCleanupDelegate::PrivacyCleanupDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

PrivacyCleanupDelegate::~PrivacyCleanupDelegate()
{
    KLOG_DEBUG() << "The PrivacyCleanupDelegate is deleted.";
}

void PrivacyCleanupDelegate::paint(QPainter *painter,
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

    if (index.column() == PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_CHECKBOX)
    {
        auto checkboxOption = option;
        initStyleOption(&checkboxOption, index);

        QStyleOptionButton checkboxStyle;
        QPixmap pixmap;
        auto value = index.model()->data(index, Qt::EditRole).toBool();
        pixmap.load(value ? ":/images/checkbox-checked-normal" : ":/images/checkbox-unchecked-normal");
        checkboxStyle.state = value ? QStyle::State_On : QStyle::State_Off;
        checkboxStyle.state |= QStyle::State_Enabled;
        checkboxStyle.iconSize = QSize(20, 20);
        checkboxStyle.rect = option.rect;
        checkboxStyle.rect.setX(option.rect.x() + 22);

        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        //        style->drawControl(QStyle::CE_CheckBox, &checkboxStyle, painter);
        style->drawItemPixmap(painter, option.rect, Qt::AlignCenter, pixmap);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool PrivacyCleanupDelegate::editorEvent(QEvent *event,
                                         QAbstractItemModel *model,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index)
{
    auto docorationRect = option.rect;
    auto mouseEvent = static_cast<QMouseEvent *>(event);

    if (event->type() == QEvent::MouseButtonPress &&
        docorationRect.contains(mouseEvent->pos()) &&
        index.column() == PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_CHECKBOX)
    {
        auto value = model->data(index, Qt::EditRole).toBool();
        model->setData(index, !value, Qt::EditRole);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

PrivacyCleanupFilterModel::PrivacyCleanupFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool PrivacyCleanupFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    for (auto i = 0; i < PRIVACY_CLEANUP_TABLE_FIELD_LAST; ++i)
    {
        auto index = sourceModel()->index(sourceRow, i, sourceParent);
        auto text = sourceModel()->data(index).toString();
        RETURN_VAL_IF_TRUE(text.contains(filterRegExp()), true);
    }

    return false;
}

PrivacyCleanupModel::PrivacyCleanupModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int PrivacyCleanupModel::rowCount(const QModelIndex &parent) const
{
    return m_infos.size();
}

int PrivacyCleanupModel::columnCount(const QModelIndex &parent) const
{
    return PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_LAST;
}

QVariant PrivacyCleanupModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

    if (index.row() >= m_infos.size() || index.column() >= PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_LAST)
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
        case PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_NUMBER:
            return index.row() + 1;
        case PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_USER_NAME:
            return info.userName;
        case PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_USER_TYPE:
            return info.userType;
        default:
            break;
        }
        break;
    }
    case Qt::EditRole:
    {
        switch (index.column())
        {
        case PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_CHECKBOX:
            return info.selected;
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

QVariant PrivacyCleanupModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        case PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_NUMBER:
            return tr("Number");
        case PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_USER_NAME:
            return tr("User name");
        case PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_USER_TYPE:
            return tr("User type");
        default:
            break;
        }
        break;
    }
    case Qt::EditRole:
    {
        switch (section)
        {
        case PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_CHECKBOX:
            return QVariant();
        }
        break;
    }
    default:
        break;
    }
    return QVariant();
}

bool PrivacyCleanupModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    RETURN_VAL_IF_TRUE(index.column() != PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_CHECKBOX, false);

    m_infos[index.row()].selected = value.toBool();
    emit dataChanged(index, index);

    if (role == Qt::UserRole || role == Qt::EditRole)
    {
        checkSelectStatus();
    }
    return true;
}

Qt::ItemFlags PrivacyCleanupModel::flags(const QModelIndex &index) const
{
    RETURN_VAL_IF_TRUE(index.column() == PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_CHECKBOX, Qt::ItemFlag::ItemIsEnabled);
    return Qt::ItemFlag::NoItemFlags;
}

void ToolBox::PrivacyCleanupModel::checkSelectStatus()
{
    auto state = Qt::Unchecked;
    int selectCount = 0;
    for (int i = 0; i < m_infos.size(); ++i)
    {
        if (m_infos[i].selected)
        {
            ++selectCount;
        }
    }

    if (selectCount >= m_infos.size())
    {
        state = Qt::Checked;
    }
    else if (selectCount > 0)
    {
        state = Qt::PartiallyChecked;
    }

    emit stateChanged(state);
}

int PrivacyCleanupModel::getPrivacyCleanupInfosSize()
{
    return m_infos.size();
}

QStringList PrivacyCleanupModel::getCheckedUserName()
{
    QStringList list;
    for (auto info : m_infos)
    {
        CONTINUE_IF_TRUE(!info.selected);
        list << info.userName;
    }
    return list;
}

void PrivacyCleanupModel::setInfos(const QList<PrivacyCleanupInfo> &infos)
{
    beginResetModel();
    SCOPE_EXIT({
        endResetModel();
    });
    m_infos.clear();
    m_infos = infos;
    emit tableUpdated(m_infos.size());
}

void ToolBox::PrivacyCleanupModel::delcheckedInfos()
{
    beginResetModel();
    SCOPE_EXIT({
        endResetModel();
    });
    auto i = -1;
    for (auto info : m_infos)
    {
        i++;
        CONTINUE_IF_TRUE(!info.selected);
        m_infos.removeAt(i);
        i--;
    }

    checkSelectStatus();
    emit tableUpdated(m_infos.size());
}

PrivacyCleanupTable::PrivacyCleanupTable(QWidget *parent)
    : QTableView(parent),
      m_filterProxy(nullptr)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_dbusProxy = new ToolBoxDbusProxy(SSR_DBUS_NAME,
                                       SSR_TOOL_BOX_DBUS_OBJECT_PATH,
                                       QDBusConnection::systemBus(),
                                       this);
    initTable();
}

void PrivacyCleanupTable::setSearchText(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;
    m_filterProxy->setFilterFixedString(text);
}

void KS::ToolBox::PrivacyCleanupTable::cleanCheckedUsers()
{
    auto checkedUserName = m_model->getCheckedUserName();
    if (checkedUserName.isEmpty())
    {
        POPUP_MESSAGE_DIALOG(tr("Please selecte items."))
        return;
    }
    auto reply = m_dbusProxy->RemoveUser(checkedUserName);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    RETURN_IF_TRUE(reply.isError());
    m_model->setInfos(getTableInfos());
    m_model->delcheckedInfos();
}

void KS::ToolBox::PrivacyCleanupTable::initTable()
{
    // 设置Model
    m_model = new PrivacyCleanupModel(this);
    m_model->setInfos(getTableInfos());
    m_headerViewProxy = new TableHeaderProxy(this);
    setHorizontalHeader(m_headerViewProxy);
    setMouseTracking(true);
    connect(m_headerViewProxy, &TableHeaderProxy::toggled, this, &PrivacyCleanupTable::checkedAllItem);
    connect(m_model, &PrivacyCleanupModel::stateChanged, m_headerViewProxy, &TableHeaderProxy::setCheckState);
    connect(m_model, &PrivacyCleanupModel::tableUpdated, this, &PrivacyCleanupTable::tableUpdated);

    m_filterProxy = new PrivacyCleanupFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);
    setShowGrid(false);

    // 设置Delegate
    setItemDelegate(new PrivacyCleanupDelegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_CHECKBOX, 50);
    m_headerViewProxy->resizeSection(PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_NUMBER, 100);
    m_headerViewProxy->resizeSection(PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_USER_NAME, 250);
    m_headerViewProxy->resizeSection(PrivacyCleanupTableField::PRIVACY_CLEANUP_TABLE_FIELD_USER_TYPE, 300);
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

QList<KS::ToolBox::PrivacyCleanupInfo> KS::ToolBox::PrivacyCleanupTable::getTableInfos()
{
    QList<KS::ToolBox::PrivacyCleanupInfo> infos;
    auto reply = m_dbusProxy->GetAllUsers();
    CHECK_ERROR_FOR_DBUS_REPLY(reply);

    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(reply.value().toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files Recordrmation failed: " << jsonError.errorString();
        return infos;
    }
    for (auto json : jsonDoc.array())
    {
        PrivacyCleanupInfo info{
            .selected = false,
            .userName = json.toObject().value(USER_NAME_JSON_KEY).toString(),
            .userType = json.toObject().value(USER_TYPE_JSON_KEY).toInt() == 0 ? tr("Manager user") : tr("Normat user")};
        infos << info;
    }
    return infos;
}

int PrivacyCleanupTable::getPrivacyCleanupInfosSize()
{
    return m_model->getPrivacyCleanupInfosSize();
}

void ToolBox::PrivacyCleanupTable::checkedAllItem(Qt::CheckState checkState)
{
    for (int i = 0; i < selectionModel()->model()->rowCount(); i++)
    {
        // 取到该行的序号列
        auto number = selectionModel()->model()->data(model()->index(i, 1)).toInt();
        auto index = m_model->index(number - 1, 0);
        m_model->setData(index, checkState == Qt::Checked, Qt::CheckStateRole);
    }
}

}  // namespace ToolBox
}  // namespace KS
