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

#include "tp-kernel-table.h"
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
#include "src/ui/tp/tp-delegate.h"
#include "src/ui/tp_proxy.h"

namespace KS
{
enum KernelField
{
    KERNEL_FIELD_CHECKBOX = 0,
    KERNEL_FIELD_NUMBER,
    KERNEL_FIELD_FILE_PATH,
    KERNEL_FIELD_STATUS,
    //    TRUSTED_FIELD_PROHIBIT_UNLOAD,
    KERNEL_FIELD_LAST
};

// 内核保护列数
#define KERNEL_TABLE_COL 4
// 执行保护列数
#define EXECUTE_TABLE_COL 5
#define KSS_JSON_KEY_DATA KSC_JK_DATA
#define KSS_JSON_KEY_DATA_PATH KSC_JK_DATA_PATH
#define KSS_JSON_KEY_DATA_TYPE KSC_JK_DATA_TYPE
#define KSS_JSON_KEY_DATA_STATUS KSC_JK_DATA_STATUS
#define KSS_JSON_KEY_DATA_HASH KSC_JK_DATA_HASH

TPKernelFilterModel::TPKernelFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool TPKernelFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QString textComb;
    for (auto i = 0; i < KERNEL_TABLE_COL; ++i)
    {
        auto index = sourceModel()->index(sourceRow, i, sourceParent);
        auto text = sourceModel()->data(index).toString();
        RETURN_VAL_IF_TRUE(text.contains(filterRegExp()), true);
    }

    return false;
}

TPKernelModel::TPKernelModel(QObject *parent) : QAbstractTableModel(parent),
                                                m_tpDBusProxy(nullptr)
{
    m_tpDBusProxy = new TPProxy(KSC_DBUS_NAME,
                                KSC_TP_DBUS_OBJECT_PATH,
                                QDBusConnection::systemBus(),
                                this);
    updateRecord();
}

int TPKernelModel::rowCount(const QModelIndex &parent) const
{
    return m_kernelRecords.size();
}

int TPKernelModel::columnCount(const QModelIndex &parent) const
{
    return KERNEL_TABLE_COL;
}

QVariant TPKernelModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

    if (index.row() >= m_kernelRecords.size() || index.column() >= KERNEL_TABLE_COL)
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QVariant();
    }

    auto trustedRecord = m_kernelRecords[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (index.column())
        {
        case KernelField::KERNEL_FIELD_NUMBER:
            return index.row() + 1;
            break;
        case KernelField::KERNEL_FIELD_FILE_PATH:
            return trustedRecord.filePath;
            break;
        case KernelField::KERNEL_FIELD_STATUS:
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
        case KernelField::KERNEL_FIELD_STATUS:
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

QVariant TPKernelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    RETURN_VAL_IF_TRUE(orientation == Qt::Orientation::Vertical, QVariant())

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (section)
        {
        case KernelField::KERNEL_FIELD_NUMBER:
            return tr("Number");
        case KernelField::KERNEL_FIELD_FILE_PATH:
            return tr("File path");
        case KernelField::KERNEL_FIELD_STATUS:
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

bool TPKernelModel::setData(const QModelIndex &index,
                            const QVariant &value,
                            int role)
{
    RETURN_VAL_IF_TRUE(index.column() != 0, false)

    m_kernelRecords[index.row()].selected = value.toBool();
    emit dataChanged(index, index);

    if (role == Qt::UserRole || role == Qt::EditRole)
    {
        onSingleStateChanged();
    }

    return true;
}

Qt::ItemFlags TPKernelModel::flags(const QModelIndex &index) const
{
    RETURN_VAL_IF_TRUE(index.column() == 0, Qt::ItemFlag::ItemIsEnabled)

    return Qt::ItemFlag::NoItemFlags;
}

void TPKernelModel::updateRecord()
{
    beginResetModel();
    m_kernelRecords.clear();
    auto reply = m_tpDBusProxy->GetModuleFiles();
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
                                            .status = status,
                                            .md5 = data.value(KSS_JSON_KEY_DATA_HASH).toString()};
            m_kernelRecords.push_back(fileRecord);
        }
    }
    endResetModel();
}

QList<TrustedRecord> TPKernelModel::getKernelRecords()
{
    return m_kernelRecords;
}

void TPKernelModel::onStateChanged(Qt::CheckState checkState)
{
    QModelIndex index;
    for (int i = 0; i < rowCount(); ++i)
    {
        index = this->index(i, 0);
        setData(index, checkState == Qt::Checked, Qt::CheckStateRole);
    }
}

void TPKernelModel::onSingleStateChanged()
{
    auto state = Qt::Unchecked;
    int selectCount = 0;
    for (int i = 0; i < m_kernelRecords.size(); ++i)
    {
        if (m_kernelRecords[i].selected)
        {
            ++selectCount;
        }
    }

    if (selectCount >= m_kernelRecords.size())
    {
        state = Qt::Checked;
    }
    else if (selectCount > 0)
    {
        state = Qt::PartiallyChecked;
    }

    emit stateChanged(state);
}

TPKernelTable::TPKernelTable(QWidget *parent) : QTableView(parent),
                                                m_filterProxy(nullptr)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置Model
    m_model = new TPKernelModel(this);
    m_headerViewProxy = new TPTableHeaderProxy(this);
    setHorizontalHeader(m_headerViewProxy);
    setMouseTracking(true);

    connect(m_headerViewProxy, &TPTableHeaderProxy::toggled, m_model, &TPKernelModel::onStateChanged);
    connect(m_model, &TPKernelModel::stateChanged, m_headerViewProxy, &TPTableHeaderProxy::setCheckState);
    m_filterProxy = new TPKernelFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);

    setShowGrid(false);
    // 设置Delegate
    setItemDelegate(new TPDelegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(KernelField::KERNEL_FIELD_CHECKBOX, 50);
    m_headerViewProxy->resizeSection(KernelField::KERNEL_FIELD_NUMBER, 100);
    m_headerViewProxy->resizeSection(KernelField::KERNEL_FIELD_FILE_PATH, 450);
    m_headerViewProxy->resizeSection(KernelField::KERNEL_FIELD_STATUS, 100);

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

    connect(this, &TPKernelTable::entered, this, &TPKernelTable::showDetails);
    connect(this, &TPKernelTable::clicked, this, &TPKernelTable::showDetails);
}

void TPKernelTable::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    m_filterProxy->setFilterFixedString(text);
}

void TPKernelTable::updateRecord()
{
    m_model->updateRecord();
}

QList<TrustedRecord> TPKernelTable::getKernelRecords()
{
    return m_model->getKernelRecords();
}

int TPKernelTable::getKerneltamperedNums()
{
    int kerneltamperedNums = 0;
    for (auto kernelRecord : m_model->getKernelRecords())
    {
        if (kernelRecord.status != tr("Certified"))
        {
            kerneltamperedNums++;
        }
    }
    return kerneltamperedNums;
}

void TPKernelTable::showDetails(const QModelIndex &index)
{
    RETURN_IF_TRUE(index.column() != KernelField::KERNEL_FIELD_FILE_PATH)

    auto module = selectionModel()->model()->data(index);
    QToolTip::showText(QCursor::pos(), QString(tr("%1")).arg(module.toString()), this);
}

}  // namespace KS
