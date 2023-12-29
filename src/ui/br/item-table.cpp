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

#include "item-table.h"

#include <QHelpEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>
#include "include/ssr-marcos.h"
#include "src/ui/br/progress.h"
#include "src/ui/br/utils.h"
#include "src/ui/common/table/table-header-proxy.h"

namespace KS
{
namespace BR
{
// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

// 分类控件
ItemTable::ItemTable(QWidget *parent)
    : QTreeView(parent),
      m_model(nullptr)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionMode(QAbstractItemView::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);
    setRootIsDecorated(false);
    m_model = new QStandardItemModel(this);
    setModel(m_model);
    setItemDelegate(new ItemTableDelegate(this));

    connect(this, &ItemTable::clicked, this, &ItemTable::setExpandItem);
    connect(this, &ItemTable::doubleClicked, this, &ItemTable::doubleClickItem);
    connect(this, SIGNAL(entered(QModelIndex)), this, SLOT(showTail(QModelIndex)));
    connect(m_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(setHeaderState(QStandardItem *)));
    // 监听item展开和关闭事件，设置相对应的箭头图片
    connect(this, &ItemTable::expanded, this, &ItemTable::setItemArrow);
    connect(this, &ItemTable::collapsed, this, &ItemTable::setItemArrow);
}

QSize ItemTable::sizeHint() const
{
    return QSize(750, 350);
}

void ItemTable::setIcon(const QList<Category *> &list, int i)
{
    // TODO 这里有点问题 所有model的图片都是一样的，后续可能更换图标
    // 需要根据图标名字来确认使用的图标
    auto iconName = QString(":/images/%1.png").arg(list.at(i)->getIconName());
    m_model->item(i)->setIcon(QIcon(":/images/ksg-category.png"));
}

void ItemTable::setItemArrow(const QModelIndex &model)
{
    QPixmap pixmap(isExpanded(model) ? ":/images/arrow-up" : ":/images/arrow-down");
    pixmap.scaled(10, 8);
    m_model->setItem(model.row(), 3, new QStandardItem(QIcon(pixmap), ""));
}

void ItemTable::initHeader()
{
    m_headerProxy = new TableHeaderProxy(this);
    setHeader(m_headerProxy);
    connect(m_headerProxy, SIGNAL(toggled(Qt::CheckState)), this, SLOT(selectAllItem(Qt::CheckState)));
    this->setHeaderHidden(false);
    m_model->setHorizontalHeaderLabels(QStringList()
                                       << QString(tr("Reinforcement Item"))
                                       << QString(tr("Info"))
                                       << QString(tr("State"))
                                       << QString(""));

    m_headerProxy->setFixedHeight(24);
    m_headerProxy->resizeSection(0, 250);
    m_headerProxy->resizeSection(1, 500);
    m_headerProxy->resizeSection(2, 90);
    m_headerProxy->resizeSection(3, 10);
    m_headerProxy->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_headerProxy->setStretchLastSection(true);
}

// 根据获取的数据，设置分类列表
void ItemTable::setItem(const QList<Category *> &list)
{
    initHeader();
    for (int i = 0; i < list.length(); ++i)
    {
        list.at(i)->setRow(i);

        m_model->setItem(i, 0, new QStandardItem(list.at(i)->getLabel()));
        m_model->setItem(i, 1, new QStandardItem(""));
        m_model->setItem(i, 2, new QStandardItem("-"));
        QPixmap pixmap(":/images/arrow-down");
        pixmap.scaled(10, 8);
        m_model->setItem(i, 3, new QStandardItem(QIcon(pixmap), ""));

        setIcon(list, i);
        m_model->item(i)->setCheckable(true);
        m_model->item(i)->setAutoTristate(true);
        m_model->item(i, 2)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_model->item(i, 3)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        CONTINUE_IF_TRUE(list.at(i)->getReinforcementItem().length() == 0)

        auto reinforcementItem = list.at(i)->getReinforcementItem();
        for (int j = 0; j < list.at(i)->getReinforcementItem().length(); ++j)
        {
            CONTINUE_IF_TRUE(reinforcementItem.at(j)->getName() == "external-hosts-login-limit" && !QFile::exists("/etc/hosts.allow"))
            auto labelItem = new QStandardItem(reinforcementItem.at(j)->getLabel());
            auto descriptionItem = new QStandardItem(reinforcementItem.at(j)->getDescription());
            auto stateItem = new QStandardItem("-");
            labelItem->setCheckable(true);

            m_model->item(i)->appendRow(labelItem);
            m_model->item(i)->setChild(labelItem->index().row(), 1, descriptionItem);
            m_model->item(i)->setChild(labelItem->index().row(), 2, stateItem);
            m_model->item(i)->setChild(labelItem->index().row(), 3, new QStandardItem(""));
            stateItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
}

void ItemTable::setHeaderState(QStandardItem *x)
{
    RETURN_IF_TRUE(x == nullptr)
    RETURN_IF_TRUE(!x->isCheckable())
    checkChanged(x);

    auto unCheckedCount = 0;
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        CONTINUE_IF_TRUE(m_model->item(i)->checkState() != Qt::Unchecked)
        unCheckedCount++;
    }

    if (unCheckedCount == m_model->rowCount())
    {
        m_headerProxy->setCheckState(Qt::Unchecked);
    }
    else if (unCheckedCount == 0)
    {
        m_headerProxy->setCheckState(Qt::Checked);
    }
    else
    {
        m_headerProxy->setCheckState(Qt::PartiallyChecked);
    }

    update();
}

/*
QTreeView在QStandardItemModel设置复选框后，并不是按照规则的，需要通过代码设置
三态复选框主要体现在树形控件里，如果子项目全选，父级需要全选，如果子项目部分选择，父级就是不完全选
*/
/*
item checkbox单击响应函数
*/
void ItemTable::checkChanged(QStandardItem *item)
{
    // 如果item是存在复选框的，那么就进行下面的操作
    Qt::CheckState state = item->checkState();  // 获取当前的选择状态
    if (item->isAutoTristate())
    {
        // 如果item是三态的，说明可以对子目录进行全选和全不选的设置
        if (state != Qt::PartiallyChecked)
        {
            // 当前是选中状态，需要对其子项目设置为全选
            checkAllChild(item, state == Qt::Checked ? true : false);
        }
    }
    else
    {
        // 说明是两态的，两态会对父级的三态有影响
        // 判断兄弟节点的情况
        checkChildChanged(item);
    }
}

//
// \brief 递归设置所有的子项目为全选或全不选状态
// \param item 当前项目
// \param check true时为全选，false时全不选
//
void ItemTable::checkAllChild(QStandardItem *item, bool check)
{
    RETURN_IF_TRUE(item == nullptr)
    auto rowCount = item->rowCount();
    for (int i = 0; i < rowCount; ++i)
    {
        auto childItems = item->child(i);
        checkAllChildRecursion(childItems, check);
    }

    if (item->isCheckable())
    {
        item->setCheckState(check ? Qt::Checked : Qt::Unchecked);
    }
}

void ItemTable::checkAllChildRecursion(QStandardItem *item, bool check)
{
    RETURN_IF_TRUE(item == nullptr)
    auto rowCount = item->rowCount();
    for (int i = 0; i < rowCount; ++i)
    {
        auto childItems = item->child(i);
        checkAllChildRecursion(childItems, check);
    }
    if (item->isCheckable())
    {
        item->setCheckState(check ? Qt::Checked : Qt::Unchecked);
    }
}

//
// \brief 根据子节点的改变，更改父节点的选择情况
// \param item
//
// 此函数也是一个递归函数，首先要判断的是父级是否到达顶层
// ，到达底层作为递归的结束，然后通过函数checkSibling判断当前的兄弟节点的具体情况
//
void ItemTable::checkChildChanged(QStandardItem *item)
{
    RETURN_IF_TRUE(item == nullptr)
    auto siblingState = checkSibling(item);
    auto parentItem = item->parent();
    RETURN_IF_TRUE(nullptr == parentItem)
    if (!parentItem->isCheckable())
    {
        checkChildChanged(parentItem);
        return;
    }

    if (Qt::PartiallyChecked == siblingState)
    {
        if (parentItem->isAutoTristate())
            parentItem->setCheckState(Qt::PartiallyChecked);
    }
    else if (Qt::Checked == siblingState)
    {
        parentItem->setCheckState(Qt::Checked);
    }
    else
    {
        parentItem->setCheckState(Qt::Unchecked);
    }
    checkChildChanged(parentItem);
}

//
// \brief 测量兄弟节点的情况，如果都选中返回Qt::Checked,都不选中Qt::Unchecked,
// \不完全选中返回Qt::PartiallyChecked
// \param item
// \return
Qt::CheckState ItemTable::checkSibling(QStandardItem *item)
{
    // 先通过父节点获取兄弟节点
    auto parent = item->parent();
    if (nullptr == parent)
    {
        return item->checkState();
    }
    int brotherCount = parent->rowCount();
    int checkedCount(0), unCheckedCount(0);

    Qt::CheckState state;
    for (int i = 0; i < brotherCount; ++i)
    {
        auto siblingItem = parent->child(i);
        state = siblingItem->checkState();
        if (Qt::PartiallyChecked == state)
        {
            return Qt::PartiallyChecked;
        }
        else if (Qt::Unchecked == state)
        {
            ++unCheckedCount;
        }
        else
        {
            ++checkedCount;
        }
        if (checkedCount > 0 && unCheckedCount > 0)
        {
            return Qt::PartiallyChecked;
        }
    }
    if (unCheckedCount > 0)
    {
        return Qt::Unchecked;
    }
    return Qt::Checked;
}

// 根据勾选项合成json字串
QStringList ItemTable::getString(const QList<Category *> &list)
{
    int count = 0;
    QStringList scanStr;
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        for (int j = 0; j < m_model->item(i)->rowCount(); ++j)
        {
            QStandardItem *item = m_model->item(i)->child(j);
            bool checkStatus = Qt::Checked == item->checkState();
            list.at(i)->getReinforcementItem().at(j)->setCheckStatus(checkStatus);
            CONTINUE_IF_TRUE(!checkStatus)
            auto name = list.at(i)->getReinforcementItem().at(j)->getName();
            scanStr.append(name);
            // KLOG_DEBUG("scanStr = %s", name.toStdString().c_str());
            count++;
        }
    }
    m_count = count;
    return scanStr;
}

QStringList ItemTable::getAllString(const QList<Category *> &categories)
{
    int count = 0;
    QStringList scanStr;
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        for (int j = 0; j < m_model->item(i)->rowCount(); ++j)
        {
            auto name = categories.at(i)->getReinforcementItem().at(j)->getName();
            categories.at(i)->getReinforcementItem().at(j)->setCheckStatus(true);
            scanStr.append(name);
            count++;
        }
    }
    m_count = count;
    return scanStr;
}

int ItemTable::getCount()
{
    return m_count;
}

// 更新每项的状态
void ItemTable::updateStatus(const QList<Category *> &list)
{
    for (int i = 0; i < list.length(); ++i)
    {
        CONTINUE_IF_TRUE(list.at(i)->getReinforcementItem().length() == 0)

        auto reinforcementItem = list.at(i)->getReinforcementItem();
        for (int j = 0; j < reinforcementItem.length(); ++j)
        {
            auto stateStr = Utils::getDefault()->state2Str(reinforcementItem.at(j)->getState());
            auto stateColor = Utils::getDefault()->state2Color(reinforcementItem.at(j)->getState());
            auto item = m_model->item(i)->child(j, 2);
            item->setText(stateStr);

            QBrush brush;
            brush.setColor(stateColor);
            item->setForeground(brush);
        }
    }
}

void ItemTable::clearCheckedStatus(const QList<Category *> &list, BRReinforcementState state)
{
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        for (int j = 0; j < m_model->item(i)->rowCount(); ++j)
        {
            auto item = m_model->item(i)->child(j);
            CONTINUE_IF_TRUE(Qt::Checked != item->checkState())
            list.at(i)->getReinforcementItem().at(j)->setState(state);
        }
    }
}

void ItemTable::getProgressCount(const QList<Category *> &list, ProgressInfo &progressInfo)
{
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        for (int j = 0; j < m_model->item(i)->rowCount(); ++j)
        {
            auto state = list.at(i)->getReinforcementItem().at(j)->getState();
            if (state == BR_REINFORCEMENT_STATE_SCAN_DONE ||
                state == BR_REINFORCEMENT_STATE_REINFORCE_DONE ||
                (state & BR_REINFORCEMENT_STATE_SAFE) == BR_REINFORCEMENT_STATE_SAFE)
            {
                progressInfo.successCount += 1;
            }
            else if (state == BR_REINFORCEMENT_STATE_UNKNOWN ||
                     state == BR_REINFORCEMENT_STATE_SCAN_ERROR ||
                     state == BR_REINFORCEMENT_STATE_REINFORCE_ERROR ||
                     (state & BR_REINFORCEMENT_STATE_UNSAFE) == BR_REINFORCEMENT_STATE_UNSAFE)
            {
                progressInfo.failureCount += 1;
            }
        }
    }
}

void ItemTable::setArgChecked(const QString &matchLabel, bool checkStatus)
{
    Qt::CheckState status;
    if (checkStatus)
        status = Qt::Checked;
    else
        status = Qt::Unchecked;
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        for (int j = 0; j < m_model->item(i)->rowCount(); ++j)
        {
            auto item = m_model->item(i)->child(j);
            CONTINUE_IF_FALSE(item->text() == matchLabel)
            item->setCheckState(status);
        }
    }
}

bool ItemTable::checkedArgStatus(const QString &matchLabel)
{
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        for (int j = 0; j < m_model->item(i)->rowCount(); ++j)
        {
            auto item = m_model->item(i)->child(j);
            RETURN_VAL_IF_TRUE(item->text() == matchLabel, item->checkState() == Qt::Checked)
        }
    }
    return false;
}

QList<QString> ItemTable::checkedAllStatus()
{
    QList<QString> statusList = {};
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        for (int j = 0; j < m_model->item(i)->rowCount(); ++j)
        {
            auto item = m_model->item(i)->child(j);
            CONTINUE_IF_FALSE(item->checkState() == Qt::Checked)
            statusList.push_back(item->text());
        }
    }
    return statusList;
}

void ItemTable::setAllChecked(Qt::CheckState isChecked)
{
    m_headerProxy->setCheckState(isChecked);
    emit m_headerProxy->toggled(isChecked);
}

void ItemTable::hideCheckBox(bool isHide)
{
    m_headerProxy->hideCheckBox(isHide);
    for (int i = 0; i < m_model->rowCount(); i++)
    {
        if (isHide)
        {
            m_model->item(i)->setData(QVariant(), Qt::CheckStateRole);
        }
        m_model->item(i)->setCheckable(!isHide);
        for (int j = 0; j < m_model->item(i)->rowCount(); ++j)
        {
            auto item = m_model->item(i)->child(j);
            if (isHide)
            {
                item->setData(QVariant(), Qt::CheckStateRole);
            }
            item->setCheckable(!isHide);
        }
    }
}

void ItemTable::setAllCheckBoxEditStatus(bool isCheckBoxEdit)
{
    // 禁用复选框时将鼠标悬浮提示一并隐藏
    m_headerProxy->hideCheckBox(!isCheckBoxEdit);
    for (int i = 0; i < m_model->rowCount(); i++)
    {
        // m_model->item(i)->setFlags(m_model->item(i)->flags() & ~Qt::ItemFlag::ItemIsEnabled);
        m_model->item(i)->setCheckable(isCheckBoxEdit);
        for (int j = 0; j < m_model->item(i)->rowCount(); ++j)
        {
            auto item = m_model->item(i)->child(j);
            // item->setFlags(item->flags() & ~Qt::ItemFlag::ItemIsEnabled);
            item->setCheckable(isCheckBoxEdit);
        }
    }
}

void ItemTable::showTail(const QModelIndex &model)
{
    QToolTip::hideText();
    if (!model.parent().isValid() && (model.column() == 1) && m_model->item(model.row())->isCheckable())
    {
        QToolTip::showText(QCursor::pos(),
                           tr("Double click this column to modify the reinforcement parameters"),
                           this,
                           this->rect(),
                           2000);
    }
    emit modelEntered(model);
}

void ItemTable::setExpandItem(const QModelIndex &model)
{
    RETURN_IF_TRUE(model.column() == 0);
    // 屏蔽子项的单击事件， 后续有其它单击处理考虑，需要处理这段代码
    RETURN_IF_TRUE(m_model->parent(model).isValid())
    // 返回兄弟index，这里要设置第一列的展开状态才能被isExpanded检测到已展开
    auto index = model.siblingAtColumn(0);
    setExpanded(index, !isExpanded(index));
}

void ItemTable::doubleClickItem(const QModelIndex &model)
{
    switch (model.column())
    {
    case 1:
        emit modifyItemArgsClicked(model);
        break;
    case 3:
    {
        // 屏蔽子项的双击事件
        BREAK_IF_TRUE(m_model->parent(model).isValid())
        // 返回兄弟index，这里要设置第一列的展开状态才能被isExpanded检测到已展开
        auto index = model.siblingAtColumn(0);
        setExpanded(index, !isExpanded(index));
        break;
    }
    default:
        break;
    }
}

void ItemTable::selectAllItem(Qt::CheckState state)
{
    if (state == Qt::Checked)
    {
        for (int i = 0; i < m_model->rowCount(); ++i)
        {
            m_model->item(i)->setCheckState(Qt::Checked);
            setHeaderState(m_model->item(i));
        }
    }
    else
    {
        for (int i = 0; i < m_model->rowCount(); ++i)
        {
            m_model->item(i)->setCheckState(Qt::Unchecked);
            setHeaderState(m_model->item(i));
        }
    }
}

ItemTableDelegate::ItemTableDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize ItemTableDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(36);
    return size;
}

}  // namespace BR
}  // namespace KS
