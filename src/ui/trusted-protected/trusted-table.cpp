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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */
#include "trusted-table.h"
#include <qt5-log-i.h>
#include <QApplication>
#include <QCheckBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemDelegate>
#include <QJsonDocument>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QToolTip>
#include "sc-i.h"
#include "sc-marcos.h"
#include "src/ui/trusted_proxy.h"

namespace KS
{
enum KernelTableField
{
    KERNEL_FIELD_CHECKBOX = 0,
    KERNEL_FIELD_NUMBER,
    KERNEL_FIELD_FILE_PATH,
    KERNEL_FIELD_STATUS,
    //    TRUSTED_FIELD_PROHIBIT_UNLOAD,
    KERNEL_FIELD_LAST
};
enum ExecuteTableField
{
    EXECUTE_FIELD_CHECKBOX = 0,
    EXECUTE_FIELD_NUMBER,
    EXECUTE_FIELD_FILE_PATH,
    EXECUTE_FIELD_FILE_TYPE,
    EXECUTE_FIELD_STATUS,
    EXECUTE_FIELD_LAST
};

// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

// 内核保护列数
#define KERNEL_TABLE_COL 4
// 执行保护列数
#define EXECUTE_TABLE_COL 5

TPDelegate::TPDelegate(QObject *parent, TRUSTED_FILE_STATUS status) : QStyledItemDelegate(parent)
{
}

TPDelegate::~TPDelegate()
{
    KLOG_DEBUG() << "The TPDelegate is deleted.";
}

void TPDelegate::paint(QPainter *painter,
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

    if (index.column() == 0)
    {
        auto checkboxOption = option;
        this->initStyleOption(&checkboxOption, index);

        QStyleOptionButton checkboxStyle;
        QPixmap pix;
        auto value = index.model()->data(index, Qt::EditRole).toBool();
        pix.load(value ? ":/images/checkbox-checked-normal" : ":/images/checkbox-unchecked-normal");
        checkboxStyle.state = value ? QStyle::State_On : QStyle::State_Off;
        checkboxStyle.state |= QStyle::State_Enabled;
        checkboxStyle.iconSize = QSize(20, 20);
        checkboxStyle.rect = option.rect;
        checkboxStyle.rect.setX(option.rect.x() + 22);

        const QWidget *widget = option.widget;
        auto *style = widget ? widget->style() : QApplication::style();
        //        style->drawControl(QStyle::CE_CheckBox, &checkboxStyle, painter);
        style->drawItemPixmap(painter, option.rect, Qt::AlignCenter, pix);
    }
    else
    {
        this->QStyledItemDelegate::paint(painter, option, index);
    }
}

bool TPDelegate::editorEvent(QEvent *event,
                             QAbstractItemModel *model,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index)
{
    auto docorationRect = option.rect;
    auto mouseEvent = static_cast<QMouseEvent *>(event);

    if (event->type() == QEvent::MouseButtonPress &&
        docorationRect.contains(mouseEvent->pos()) &&
        index.column() == 0)
    {
        auto value = model->data(index, Qt::EditRole).toBool();
        model->setData(index, !value, Qt::EditRole);
    }

    return this->QStyledItemDelegate::editorEvent(event, model, option, index);
}

TPFilterModel::TPFilterModel(QObject *parent, TRUSTED_PROTECT_TYPE type) : QSortFilterProxyModel(parent),
                                                                           m_type(type)
{
}

bool TPFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QString textComb;
    auto col = (m_type == TRUSTED_PROTECT_TYPE::KERNEL_PROTECT) ? KERNEL_TABLE_COL : EXECUTE_TABLE_COL;
    for (auto i = 0; i < col; ++i)
    {
        auto index = this->sourceModel()->index(sourceRow, i, sourceParent);
        auto text = this->sourceModel()->data(index).toString();
        RETURN_VAL_IF_TRUE(text.contains(this->filterRegExp()), true);
    }

    return false;
}

TPModel::TPModel(QObject *parent, TRUSTED_PROTECT_TYPE type) : QAbstractTableModel(parent),
                                                               m_type(type)
{
    this->m_trustedProtectedProxy = new TrustedProxy(SC_DBUS_NAME,
                                                     SC_TRUSTED_PROTECTED_DBUS_OBJECT_PATH,
                                                     QDBusConnection::systemBus(),
                                                     this);
    this->updateInfo();
}

int TPModel::rowCount(const QModelIndex &parent) const
{
    return this->m_trustedInfos.size();
}

int TPModel::columnCount(const QModelIndex &parent) const
{
    return (m_type == TRUSTED_PROTECT_TYPE::KERNEL_PROTECT) ? KERNEL_TABLE_COL : EXECUTE_TABLE_COL;
}

QVariant TPModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());
    auto col = (m_type == TRUSTED_PROTECT_TYPE::KERNEL_PROTECT) ? KERNEL_TABLE_COL : EXECUTE_TABLE_COL;

    if (index.row() >= this->m_trustedInfos.size() || index.column() >= col)
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QVariant();
    }

    auto trustedInfo = this->m_trustedInfos[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
    {
        if (m_type == TRUSTED_PROTECT_TYPE::KERNEL_PROTECT)
        {
            switch (index.column())
            {
            case KernelTableField::KERNEL_FIELD_NUMBER:
                return index.row() + 1;
                break;
            case KernelTableField::KERNEL_FIELD_FILE_PATH:
                return trustedInfo.filePath;
                break;
            case KernelTableField::KERNEL_FIELD_STATUS:
                return trustedInfo.status;
                break;
            default:
                break;
            }
        }
        else
        {
            switch (index.column())
            {
            case ExecuteTableField::EXECUTE_FIELD_NUMBER:
                return index.row() + 1;
                break;
            case ExecuteTableField::EXECUTE_FIELD_FILE_PATH:
                return trustedInfo.filePath;
                break;
            case ExecuteTableField::EXECUTE_FIELD_FILE_TYPE:
                return trustedInfo.type;
                break;
            case ExecuteTableField::EXECUTE_FIELD_STATUS:
                return trustedInfo.status;
                break;
            default:
                break;
            }
        }
    }
    case Qt::EditRole:
    {
        switch (index.column())
        {
        case 0:
            return trustedInfo.selected;
            break;
        default:
            break;
        }
    }
    case Qt::TextColorRole:
    {
        if (m_type == TRUSTED_PROTECT_TYPE::KERNEL_PROTECT)
        {
            switch (index.column())
            {
            case KernelTableField::KERNEL_FIELD_STATUS:
            {
                if (trustedInfo.status == tr("Certified"))
                {
                    QBrush brush((QColor(0, 162, 255)));  // #00a2ff
                    brush.setStyle(Qt::SolidPattern);
                    return brush;
                }
                else
                {
                    QBrush brush((QColor(Qt::red)));
                    brush.setStyle(Qt::SolidPattern);
                    return brush;
                }
                break;
            }
            default:
                break;
            }
        }
        else
        {
            switch (index.column())
            {
            case ExecuteTableField::EXECUTE_FIELD_STATUS:
            {
                if (trustedInfo.status == tr("Certified"))
                {
                    QBrush brush((QColor(0, 162, 255)));  // #00a2ff
                    brush.setStyle(Qt::SolidPattern);
                    return brush;
                }
                else
                {
                    QBrush brush((QColor(Qt::red)));
                    brush.setStyle(Qt::SolidPattern);
                    return brush;
                }
                break;
            }
            default:
                break;
            }
        }
    }
    default:
        break;
    }

    return QVariant();
}

QVariant TPModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Vertical)
    {
        return QVariant();
    }
    switch (role)
    {
    case Qt::DisplayRole:
    {
        if (m_type == TRUSTED_PROTECT_TYPE::KERNEL_PROTECT)
        {
            switch (section)
            {
            case KernelTableField::KERNEL_FIELD_NUMBER:
                return tr("Number");
            case KernelTableField::KERNEL_FIELD_FILE_PATH:
                return tr("File path");
            case KernelTableField::KERNEL_FIELD_STATUS:
                return tr("Status");
            default:
                break;
            }
        }
        else
        {
            switch (section)
            {
            case ExecuteTableField::EXECUTE_FIELD_NUMBER:
                return tr("Number");
            case ExecuteTableField::EXECUTE_FIELD_FILE_PATH:
                return tr("File path");
            case ExecuteTableField::EXECUTE_FIELD_FILE_TYPE:
                return tr("Type");
            case ExecuteTableField::EXECUTE_FIELD_STATUS:
                return tr("Status");
            default:
                break;
            }
        }
    }
    default:
        break;
    }
    return QVariant();
}

bool TPModel::setData(const QModelIndex &index,
                      const QVariant &value,
                      int role)
{
    if (index.column() != 0)
    {
        return false;
    }

    this->m_trustedInfos[index.row()].selected = value.toBool();
    emit dataChanged(index, index);

    if (role == Qt::UserRole || role == Qt::EditRole)
    {
        onSingleStateChanged();
    }

    return true;
}

Qt::ItemFlags TPModel::flags(const QModelIndex &index) const
{
    if (index.column() == 0)
    {
        return Qt::ItemFlag::ItemIsEnabled;
    }
    return Qt::ItemFlag::NoItemFlags;
}

void TPModel::updateInfo()
{
    beginResetModel();
    m_trustedInfos.clear();
    auto reply = (m_type == TRUSTED_PROTECT_TYPE::KERNEL_PROTECT) ? this->m_trustedProtectedProxy->GetModuleFiles() : this->m_trustedProtectedProxy->GetExecuteFiles();
    auto files = reply.value();

    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(files.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files information failed: " << jsonError.errorString();
    }
    else
    {
        // 后台返回数据需先转为obj后，将obj中的data字段转为arr
        auto jsonDataArray = jsonDoc.object().value(KSS_JSON_KEY_DATA).toArray();

        for (auto jsonData : jsonDataArray)
        {
            auto data = jsonData.toObject();
            QString type;
            QString status;
            switch (data.value(KSS_JSON_KEY_DATA_TYPE).toInt())
            {
            case TRUSTED_FILE_TYPE::UNKNOWN_TYPE:
                type = QString(tr("Unknown file"));
                break;
            case TRUSTED_FILE_TYPE::EXECUTABLE_FILE:
                type = QString(tr("Executable file"));
                break;
            case TRUSTED_FILE_TYPE::DYNAMIC_LIBRARY:
                type = QString(tr("Dynamic library"));
                break;
            case TRUSTED_FILE_TYPE::KERNEL_MODULE:
                type = QString(tr("Kernel file"));
                break;
            case TRUSTED_FILE_TYPE::EXECUTABLE_SCRIPT:
                type = QString(tr("Executable script"));
                break;
            default:
                break;
            }

            if (data.value(KSS_JSON_KEY_DATA_STATUS).toInt() == TRUSTED_FILE_STATUS::NORMAL_STATUS)
            {
                status = QString(tr("Certified"));
            }
            else
            {
                status = QString(tr("Being tampered with"));
            }
            auto fileInfo = TrustedInfo{.selected = false,
                                        .filePath = data.value(KSS_JSON_KEY_DATA_PATH).toString(),
                                        .type = type,
                                        .status = status};
            this->m_trustedInfos.push_back(fileInfo);
        }
    }
    endResetModel();
}

QList<TrustedInfo> TPModel::getTrustedInfos()
{
    return this->m_trustedInfos;
}

void TPModel::onStateChanged(Qt::CheckState checkState)
{
    QModelIndex index;
    for (int i = 0; i < rowCount(); ++i)
    {
        index = this->index(i, 0);
        this->setData(index, checkState == Qt::Checked, Qt::CheckStateRole);
    }
}

void TPModel::onSingleStateChanged()
{
    auto state = Qt::Unchecked;
    int selectCount = 0;
    for (int i = 0; i < m_trustedInfos.size(); ++i)
    {
        if (m_trustedInfos[i].selected)
        {
            ++selectCount;
        }
    }

    if (selectCount >= m_trustedInfos.size())
    {
        state = Qt::Checked;
    }
    else if (selectCount > 0)
    {
        state = Qt::PartiallyChecked;
    }

    emit this->stateChanged(state);
}

TPTable::TPTable(QWidget *parent, TRUSTED_PROTECT_TYPE type) : QTableView(parent),
                                                               m_filterProxy(nullptr)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置Model
    m_model = new TPModel(this, type);
    m_newHeaderView = new NewHeaderView(this);
    this->setHorizontalHeader(m_newHeaderView);
    this->setMouseTracking(true);

    connect(m_newHeaderView, &NewHeaderView::toggled, m_model, &TPModel::onStateChanged);
    connect(m_model, &TPModel::stateChanged, m_newHeaderView, &NewHeaderView::setCheckState);
    this->m_filterProxy = new TPFilterModel(this, type);
    this->m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    this->setModel(this->m_filterProxy);

    this->setShowGrid(false);
    // 设置Delegate
    this->setItemDelegate(new TPDelegate(this));

    // 设置水平行表头
    if (type == TRUSTED_PROTECT_TYPE::KERNEL_PROTECT)
    {
        m_newHeaderView->resizeSection(KernelTableField::KERNEL_FIELD_CHECKBOX, 50);
        m_newHeaderView->resizeSection(KernelTableField::KERNEL_FIELD_NUMBER, 100);
        m_newHeaderView->resizeSection(KernelTableField::KERNEL_FIELD_FILE_PATH, 450);
        m_newHeaderView->resizeSection(KernelTableField::KERNEL_FIELD_STATUS, 100);
    }
    else
    {
        m_newHeaderView->resizeSection(ExecuteTableField::EXECUTE_FIELD_CHECKBOX, 50);
        m_newHeaderView->resizeSection(ExecuteTableField::EXECUTE_FIELD_NUMBER, 100);
        m_newHeaderView->resizeSection(ExecuteTableField::EXECUTE_FIELD_FILE_PATH, 350);
        m_newHeaderView->resizeSection(ExecuteTableField::EXECUTE_FIELD_FILE_TYPE, 100);
        m_newHeaderView->resizeSection(ExecuteTableField::EXECUTE_FIELD_STATUS, 100);
    }
    m_newHeaderView->setStretchLastSection(true);
    //    m_newHeaderView->set(false);
    m_newHeaderView->setSectionsMovable(false);
    m_newHeaderView->setDefaultAlignment(Qt::AlignLeft);
    m_newHeaderView->setFixedHeight(24);
    m_newHeaderView->setDefaultAlignment(Qt::AlignVCenter);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);

    connect(this, &TPTable::entered, this, &TPTable::mouseEnter);
}

void TPTable::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    m_filterProxy->setFilterFixedString(text);
}

void TPTable::updateInfo()
{
    m_model->updateInfo();
}

QList<TrustedInfo> TPTable::getTrustedInfos()
{
    return m_model->getTrustedInfos();
}

void TPTable::mouseEnter(const QModelIndex &index)
{
    if (index.column() != ExecuteTableField::EXECUTE_FIELD_FILE_PATH)
    {
        return;
    }
    auto mod = this->selectionModel()->model()->data(index);
    QToolTip::showText(QCursor::pos(), mod.toString(), this, this->rect(), 2000);
}

}  // namespace KS
