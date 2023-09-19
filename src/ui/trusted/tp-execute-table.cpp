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

#include "tp-execute-table.h"
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
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "src/ui/trusted/tp-delegate.h"
#include "src/ui/trusted_proxy.h"

namespace KS
{
enum ExecuteField
{
    EXECUTE_FIELD_CHECKBOX = 0,
    EXECUTE_FIELD_NUMBER,
    EXECUTE_FIELD_FILE_PATH,
    EXECUTE_FIELD_FILE_TYPE,
    EXECUTE_FIELD_STATUS,
    EXECUTE_FIELD_LAST
};

// 执行保护列数
#define EXECUTE_TABLE_COL 5

#define KSS_JSON_KEY_DATA KSC_JK_DATA
#define KSS_JSON_KEY_DATA_PATH KSC_JK_DATA_PATH
#define KSS_JSON_KEY_DATA_TYPE KSC_JK_DATA_TYPE
#define KSS_JSON_KEY_DATA_STATUS KSC_JK_DATA_STATUS

TPExecuteFilterModel::TPExecuteFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool TPExecuteFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QString textComb;
    for (auto i = 0; i < EXECUTE_TABLE_COL; ++i)
    {
        auto index = this->sourceModel()->index(sourceRow, i, sourceParent);
        auto text = this->sourceModel()->data(index).toString();
        RETURN_VAL_IF_TRUE(text.contains(this->filterRegExp()), true);
    }

    return false;
}

TPExecuteModel::TPExecuteModel(QObject *parent) : QAbstractTableModel(parent)
{
    this->m_trustedProtectedProxy = new TrustedProxy(KSC_DBUS_NAME,
                                                     KSC_TRUSTED_PROTECTED_DBUS_OBJECT_PATH,
                                                     QDBusConnection::systemBus(),
                                                     this);
    this->updateRecord();
}

int TPExecuteModel::rowCount(const QModelIndex &parent) const
{
    return this->m_executeRecords.size();
}

int TPExecuteModel::columnCount(const QModelIndex &parent) const
{
    return EXECUTE_TABLE_COL;
}

QVariant TPExecuteModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

    if (index.row() >= this->m_executeRecords.size() || index.column() >= EXECUTE_TABLE_COL)
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QVariant();
    }

    auto trustedRecord = this->m_executeRecords[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (index.column())
        {
        case ExecuteField::EXECUTE_FIELD_NUMBER:
            return index.row() + 1;
            break;
        case ExecuteField::EXECUTE_FIELD_FILE_PATH:
            return trustedRecord.filePath;
            break;
        case ExecuteField::EXECUTE_FIELD_FILE_TYPE:
            return trustedRecord.type;
            break;
        case ExecuteField::EXECUTE_FIELD_STATUS:
            return trustedRecord.status;
            break;
        default:
            break;
        }
    }
    case Qt::EditRole:
    {
        switch (index.column())
        {
        case 0:
            return trustedRecord.selected;
            break;
        default:
            break;
        }
    }
    case Qt::TextColorRole:
    {
        switch (index.column())
        {
        case ExecuteField::EXECUTE_FIELD_STATUS:
        {
            if (trustedRecord.status == tr("Certified"))
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
    default:
        break;
    }

    return QVariant();
}

QVariant TPExecuteModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        case ExecuteField::EXECUTE_FIELD_NUMBER:
            return tr("Number");
        case ExecuteField::EXECUTE_FIELD_FILE_PATH:
            return tr("File path");
        case ExecuteField::EXECUTE_FIELD_FILE_TYPE:
            return tr("Type");
        case ExecuteField::EXECUTE_FIELD_STATUS:
            return tr("Status");
        default:
            break;
        }
    }
    default:
        break;
    }
    return QVariant();
}

bool TPExecuteModel::setData(const QModelIndex &index,
                             const QVariant &value,
                             int role)
{
    if (index.column() != 0)
    {
        return false;
    }

    this->m_executeRecords[index.row()].selected = value.toBool();
    emit dataChanged(index, index);

    if (role == Qt::UserRole || role == Qt::EditRole)
    {
        onSingleStateChanged();
    }

    return true;
}

Qt::ItemFlags TPExecuteModel::flags(const QModelIndex &index) const
{
    if (index.column() == 0)
    {
        return Qt::ItemFlag::ItemIsEnabled;
    }
    return Qt::ItemFlag::NoItemFlags;
}

void TPExecuteModel::updateRecord()
{
    beginResetModel();
    m_executeRecords.clear();
    auto reply = this->m_trustedProtectedProxy->GetExecuteFiles();
    auto files = reply.value();

    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(files.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files Recordrmation failed: " << jsonError.errorString();
    }
    else
    {
        // 后台返回数据需先转为obj后，将obj中的data字段转为arr
        auto jsonDataArray = jsonDoc.object().value(KSS_JSON_KEY_DATA).toArray();

        for (auto jsonData : jsonDataArray)
        {
            auto data = jsonData.toObject();
            auto type = TPUtils::fileTypeEnum2Str(data.value(KSS_JSON_KEY_DATA_TYPE).toInt());
            auto status = TPUtils::fileStatusEnum2Str(data.value(KSS_JSON_KEY_DATA_STATUS).toInt());

            auto fileRecord = TrustedRecord{.selected = false,
                                            .filePath = data.value(KSS_JSON_KEY_DATA_PATH).toString(),
                                            .type = type,
                                            .status = status};
            this->m_executeRecords.push_back(fileRecord);
        }
    }
    endResetModel();
}

QList<TrustedRecord> TPExecuteModel::getExecuteRecords()
{
    return this->m_executeRecords;
}

void TPExecuteModel::onStateChanged(Qt::CheckState checkState)
{
    QModelIndex index;
    for (int i = 0; i < rowCount(); ++i)
    {
        index = this->index(i, 0);
        this->setData(index, checkState == Qt::Checked, Qt::CheckStateRole);
    }
}

void TPExecuteModel::onSingleStateChanged()
{
    auto state = Qt::Unchecked;
    int selectCount = 0;
    for (int i = 0; i < m_executeRecords.size(); ++i)
    {
        if (m_executeRecords[i].selected)
        {
            ++selectCount;
        }
    }

    if (selectCount >= m_executeRecords.size())
    {
        state = Qt::Checked;
    }
    else if (selectCount > 0)
    {
        state = Qt::PartiallyChecked;
    }

    emit this->stateChanged(state);
}

TPExecuteTable::TPExecuteTable(QWidget *parent) : QTableView(parent),
                                                  m_filterProxy(nullptr)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置Model
    m_model = new TPExecuteModel(this);
    m_headerViewProxy = new TPTableHeaderProxy(this);
    this->setHorizontalHeader(m_headerViewProxy);
    this->setMouseTracking(true);

    connect(m_headerViewProxy, &TPTableHeaderProxy::toggled, m_model, &TPExecuteModel::onStateChanged);
    connect(m_model, &TPExecuteModel::stateChanged, m_headerViewProxy, &TPTableHeaderProxy::setCheckState);
    this->m_filterProxy = new TPExecuteFilterModel(this);
    this->m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    this->setModel(this->m_filterProxy);

    this->setShowGrid(false);
    // 设置Delegate
    this->setItemDelegate(new TPDelegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(ExecuteField::EXECUTE_FIELD_CHECKBOX, 50);
    m_headerViewProxy->resizeSection(ExecuteField::EXECUTE_FIELD_NUMBER, 100);
    m_headerViewProxy->resizeSection(ExecuteField::EXECUTE_FIELD_FILE_PATH, 350);
    m_headerViewProxy->resizeSection(ExecuteField::EXECUTE_FIELD_FILE_TYPE, 100);
    m_headerViewProxy->resizeSection(ExecuteField::EXECUTE_FIELD_STATUS, 100);

    m_headerViewProxy->setStretchLastSection(true);
    //    m_headerViewProxy->set(false);
    m_headerViewProxy->setSectionsMovable(false);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignLeft);
    m_headerViewProxy->setFixedHeight(24);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignVCenter);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);

    connect(this, &TPExecuteTable::entered, this, &TPExecuteTable::mouseEnter);
}

void TPExecuteTable::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    m_filterProxy->setFilterFixedString(text);
}

void TPExecuteTable::updateRecord()
{
    m_model->updateRecord();
}

QList<TrustedRecord> TPExecuteTable::getExecuteRecords()
{
    return m_model->getExecuteRecords();
}

int TPExecuteTable::getExecutetamperedNums()
{
    int executetamperedNums = 0;
    for (auto kernelRecord : m_model->getExecuteRecords())
    {
        if (kernelRecord.status != tr("Certified"))
        {
            executetamperedNums++;
        }
    }
    return executetamperedNums;
}

void TPExecuteTable::mouseEnter(const QModelIndex &index)
{
    if (index.column() != ExecuteField::EXECUTE_FIELD_FILE_PATH)
    {
        return;
    }
    auto mod = this->selectionModel()->model()->data(index);
    QToolTip::showText(QCursor::pos(), mod.toString(), this, this->rect(), 2000);
}

}  // namespace KS
