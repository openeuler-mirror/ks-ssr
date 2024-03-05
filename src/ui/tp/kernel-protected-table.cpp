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

#include "kernel-protected-table.h"
#include <qt5-log-i.h>
#include <QAction>
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
#include "src/ui/common/table/header-button-delegate.h"
#include "src/ui/kss_dbus_proxy.h"
#include "src/ui/tp/kernel-protected-delegate.h"
#include "ssr-i.h"
#include "ssr-marcos.h"

namespace KS
{
namespace TP
{
enum KernelField
{
    KERNEL_FIELD_CHECKBOX = 0,
    KERNEL_FIELD_NUMBER,
    KERNEL_FIELD_FILE_PATH,
    KERNEL_FIELD_STATUS,
    KERNEL_FIELD_PROHIBIT_UNLOAD,
    KERNEL_FIELD_LAST
};

// 内核保护列数
#define KERNEL_TABLE_COL 5
#define KSS_JSON_KEY_DATA SSR_KSS_JK_DATA
#define KSS_JSON_KEY_DATA_PATH SSR_KSS_JK_DATA_PATH
#define KSS_JSON_KEY_DATA_TYPE SSR_KSS_JK_DATA_TYPE
#define KSS_JSON_KEY_DATA_STATUS SSR_KSS_JK_DATA_STATUS
#define KSS_JSON_KEY_DATA_HASH SSR_KSS_JK_DATA_HASH
#define KSS_JSON_KEY_DATA_GUARD SSR_KSS_JK_DATA_GUARD

KernelProtectedFilterModel::KernelProtectedFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void KernelProtectedFilterModel::setSearchText(const QString &text)
{
    m_searchText = text;
}

bool KernelProtectedFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    RETURN_VAL_IF_TRUE(filterRegExp().isEmpty(), false)
    QString sourceString;
    for (auto i = 0; i < KERNEL_TABLE_COL; ++i)
    {
        auto index = sourceModel()->index(sourceRow, i, sourceParent);
        auto text = sourceModel()->data(index).toString();
        sourceString += text;
    }

    if (!m_searchText.isEmpty())
    {
        RETURN_VAL_IF_TRUE(sourceString.contains(m_searchText) && sourceString.contains(filterRegExp()), true);
    }
    else
    {
        RETURN_VAL_IF_TRUE(sourceString.contains(filterRegExp()), true);
    }
    return false;
}

KernelProtectedModel::KernelProtectedModel(QObject *parent)
    : QAbstractTableModel(parent),
      m_tpDBusProxy(nullptr)
{
    m_tpDBusProxy = new KSSDbusProxy(SSR_DBUS_NAME,
                                     SSR_KSS_INIT_DBUS_OBJECT_PATH,
                                     QDBusConnection::systemBus(),
                                     this);
    connect(m_tpDBusProxy, &KSSDbusProxy::TrustedFilesChange, this, &KernelProtectedModel::updateRecord);

    updateRecord();
}

int KernelProtectedModel::rowCount(const QModelIndex &parent) const
{
    return m_kernelRecords.size();
}

int KernelProtectedModel::columnCount(const QModelIndex &parent) const
{
    return KERNEL_TABLE_COL;
}

QVariant KernelProtectedModel::data(const QModelIndex &index, int role) const
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
        case PROHIBIT_UNLOADING_COL:
            return trustedRecord.guard;
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

QVariant KernelProtectedModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            return "";
        case KernelField::KERNEL_FIELD_PROHIBIT_UNLOAD:
            return tr("Prohibt unload");
        default:
            break;
        }
    }
    default:
        break;
    }
    return QVariant();
}

bool KernelProtectedModel::setData(const QModelIndex &index,
                                   const QVariant &value,
                                   int role)
{
    //    RETURN_VAL_IF_TRUE(index.column() != 0, false)
    switch (index.column())
    {
    case 0:
        m_kernelRecords[index.row()].selected = value.toBool();
        emit dataChanged(index, index);
        if (role == Qt::UserRole || role == Qt::EditRole)
        {
            checkSelectStatus();
        }
        break;
    case PROHIBIT_UNLOADING_COL:
        m_kernelRecords[index.row()].guard = value.toBool();
        emit dataChanged(index, index);
        break;
    default:
        return false;
    }

    return true;
}

Qt::ItemFlags KernelProtectedModel::flags(const QModelIndex &index) const
{
    RETURN_VAL_IF_TRUE(index.column() == 0 || index.column() == PROHIBIT_UNLOADING_COL, Qt::ItemFlag::ItemIsEnabled)

    return Qt::ItemFlag::NoItemFlags;
}

void KernelProtectedModel::updateRecord()
{
    beginResetModel();
    SCOPE_EXIT(
        {
            endResetModel();
        });

    m_kernelRecords.clear();
    // 刷新时checkbox状态清空
    emit stateChanged(Qt::Unchecked);

    auto reply = m_tpDBusProxy->GetTrustedFiles(SSRKSSTrustedFileType::SSR_KSS_TRUSTED_FILE_TYPE_KERNEL);
    reply.waitForFinished();
    auto files = reply.value();

    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(files.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files Recordrmation failed: " << jsonError.errorString();
        return;
    }

    // 后台返回数据需先转为obj后，将obj中的data字段转为arr
    auto jsonDataArray = jsonDoc.object().value(KSS_JSON_KEY_DATA).toArray();
    // 倒序排序
    auto jsonData = jsonDataArray.end();
    while (jsonData != jsonDataArray.begin())
    {
        jsonData--;
        auto kssData = jsonData->toObject();
        auto type = Utils::fileTypeEnum2Str(kssData.value(KSS_JSON_KEY_DATA_TYPE).toInt());
        auto status = Utils::fileStatusEnum2Str(kssData.value(KSS_JSON_KEY_DATA_STATUS).toInt());
        auto fileRecord = TrustedRecord{.selected = false,
                                        .filePath = kssData.value(KSS_JSON_KEY_DATA_PATH).toString(),
                                        .type = type,
                                        .status = status,
                                        .md5 = kssData.value(KSS_JSON_KEY_DATA_HASH).toString(),
                                        .guard = kssData.value(KSS_JSON_KEY_DATA_GUARD).toInt() == 0 ? false : true};
        m_kernelRecords.push_back(fileRecord);
    }
    emit filesUpdate(m_kernelRecords.size());  // NOSONAR
}

QList<TrustedRecord> KernelProtectedModel::getKernelRecords()
{
    return m_kernelRecords;
}

void KernelProtectedModel::checkSelectStatus()
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

KernelProtectedTable::KernelProtectedTable(QWidget *parent)
    : QTableView(parent),
      m_filterProxy(nullptr)
{
    initTable();
    initTableHeaderButton();
}

void KernelProtectedTable::setSearchText(const QString &text)
{
    m_searchText = text;
    m_filterProxy->setSearchText(m_searchText);
    filterFixedString();
}

void KernelProtectedTable::updateInfo()
{
    m_model->updateRecord();
}

QList<TrustedRecord> KernelProtectedTable::getKernelRecords()
{
    return m_model->getKernelRecords();
}

int KernelProtectedTable::getKerneltamperedNums()
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
void KernelProtectedTable::initTable()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 设置Model
    m_model = new KernelProtectedModel(this);
    m_headerViewProxy = new TableHeaderProxy(this);
    setHorizontalHeader(m_headerViewProxy);
    setMouseTracking(true);

    connect(m_headerViewProxy, &TableHeaderProxy::toggled, this, &KernelProtectedTable::checkedAllItem);
    connect(m_model, &KernelProtectedModel::stateChanged, m_headerViewProxy, &TableHeaderProxy::setCheckState);
    connect(m_model, &KernelProtectedModel::filesUpdate, this, &KernelProtectedTable::filesUpdate);

    m_filterProxy = new KernelProtectedFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);

    setShowGrid(false);
    // 设置Delegate
    setItemDelegate(new KernelProtectedDelegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(KernelField::KERNEL_FIELD_CHECKBOX, 50);
    m_headerViewProxy->resizeSection(KernelField::KERNEL_FIELD_NUMBER, 80);
    m_headerViewProxy->resizeSection(KernelField::KERNEL_FIELD_FILE_PATH, 390);
    m_headerViewProxy->resizeSection(KernelField::KERNEL_FIELD_STATUS, 100);
    m_headerViewProxy->resizeSection(KernelField::KERNEL_FIELD_PROHIBIT_UNLOAD, 80);

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

    connect(this, &KernelProtectedTable::entered, this, &KernelProtectedTable::itemEntered);
    connect(this, &KernelProtectedTable::entered, this, [this](const QModelIndex &index)
        {
            RETURN_IF_TRUE(!index.isValid());
            RETURN_IF_TRUE(index.column() > m_model->columnCount() || index.row() > m_model->rowCount());
            // 判断内容是否显示完整
            auto itemRect = this->visualRect(index);
            // 计算文本宽度
            QFontMetrics metrics(this->font());
            auto textWidth = metrics.horizontalAdvance(m_model->data(index).toString());
            RETURN_IF_TRUE(textWidth <= itemRect.width())
            auto mod = selectionModel()->model()->data(index);
            QToolTip::showText(QCursor::pos(), mod.toString(), this, rect(), 5000);
        });
    connect(this, &KernelProtectedTable::clicked, this, &KernelProtectedTable::itemClicked);
}

void KernelProtectedTable::initTableHeaderButton()
{
    // 状态筛选
    m_statusButton = new HeaderButtonDelegate(this);
    m_statusButton->setButtonText(tr("Status"));

    auto certified = new QAction(tr("Certified"), m_statusButton);
    auto beingTamperedWith = new QAction(tr("Being tampered with"), m_statusButton);
    m_statusKeys << tr("Certified") << tr("Being tampered with");
    m_statusButton->addMenuActions(QList<QAction *>() << certified << beingTamperedWith);
    connect(m_statusButton, &HeaderButtonDelegate::menuTriggered, this, [this]()
            {
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
                }
                filterFixedString();
            });
    QMap<int, HeaderButtonDelegate *> headerButtons;
    headerButtons.insert(KERNEL_FIELD_STATUS, m_statusButton);
    m_headerViewProxy->setHeaderButtons(headerButtons);
    filterFixedString();
}

void KernelProtectedTable::filterFixedString()
{
    QStringList keys = {};
    for (auto key : m_statusKeys)
    {
        CONTINUE_IF_TRUE(key.isEmpty())
        keys << key;
    }
    QString pattern = keys.join("|");
    KLOG_DEBUG() << "The search text is change to " << pattern;
    m_filterProxy->setFilterRegExp(pattern);
}
void KernelProtectedTable::itemEntered(const QModelIndex &index)
{
    if (index.column() == KernelField::KERNEL_FIELD_PROHIBIT_UNLOAD)
    {
        this->setCursor(Qt::PointingHandCursor);
    }
    else
    {
        this->setCursor(Qt::ArrowCursor);
    }
}

void KernelProtectedTable::itemClicked(const QModelIndex &index)
{
    if (index.column() == KernelField::KERNEL_FIELD_FILE_PATH)
    {
        auto module = selectionModel()->model()->data(index);
        QToolTip::showText(QCursor::pos(), QString(tr("%1")).arg(module.toString()), this);
    }
    RETURN_IF_TRUE(index.column() != KernelField::KERNEL_FIELD_PROHIBIT_UNLOAD)
    auto module = selectionModel()->model()->data(index);
    // 取到该行的序号列
    auto number = selectionModel()->model()->data(model()->index(index.row(), 1)).toInt();
    auto kernelRecord = getKernelRecords()[number - 1];

    emit prohibitUnloadingStatusChange(module.toBool(), kernelRecord.filePath);
}

void KernelProtectedTable::checkedAllItem(Qt::CheckState checkState)
{
    for (int i = 0; i < selectionModel()->model()->rowCount(); i++)
    {
        // 取到该行的序号列
        auto number = selectionModel()->model()->data(model()->index(i, 1)).toInt();
        auto index = m_model->index(number - 1, 0);
        m_model->setData(index, checkState == Qt::Checked, Qt::CheckStateRole);
    }
}
}  // namespace TP
}  // namespace KS
