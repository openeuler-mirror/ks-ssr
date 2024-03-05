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

#include "execute-protected-table.h"
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
#include "src/ui/tp/delegate.h"
#include "ssr-i.h"
#include "ssr-marcos.h"

namespace KS
{
namespace TP
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

#define KSS_JSON_KEY_DATA SSR_KSS_JK_DATA
#define KSS_JSON_KEY_DATA_PATH SSR_KSS_JK_DATA_PATH
#define KSS_JSON_KEY_DATA_TYPE SSR_KSS_JK_DATA_TYPE
#define KSS_JSON_KEY_DATA_STATUS SSR_KSS_JK_DATA_STATUS
#define KSS_JSON_KEY_DATA_HASH SSR_KSS_JK_DATA_HASH

ExecuteProtectedFilterModel::ExecuteProtectedFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void ExecuteProtectedFilterModel::setSearchText(const QString &text)
{
    m_searchText = text;
}

bool ExecuteProtectedFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // 适用于有多个表头筛选列的情况下，正则由格式为 (表头1正则).*(表头n正则)，若没有).*(，则代表有一列是没有选中的筛选项的，表格不需要显示数据
    RETURN_VAL_IF_TRUE(filterRegExp().isEmpty() || !filterRegExp().pattern().contains(").*("), false)
    QString sourceString;
    for (auto i = 0; i < EXECUTE_TABLE_COL; ++i)
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

ExecuteProtectedModel::ExecuteProtectedModel(QObject *parent)
    : QAbstractTableModel(parent),
      m_tpDBusProxy(nullptr)
{
    m_tpDBusProxy = new KSSDbusProxy(SSR_DBUS_NAME,
                                     SSR_KSS_INIT_DBUS_OBJECT_PATH,
                                     QDBusConnection::systemBus(),
                                     this);
    connect(m_tpDBusProxy, &KSSDbusProxy::TrustedFilesChange, this, &ExecuteProtectedModel::updateRecord);
    updateRecord();
}

int ExecuteProtectedModel::rowCount(const QModelIndex &parent) const
{
    return m_executeRecords.size();
}

int ExecuteProtectedModel::columnCount(const QModelIndex &parent) const
{
    return EXECUTE_TABLE_COL;
}

QVariant ExecuteProtectedModel::data(const QModelIndex &index, int role) const
{
    RETURN_VAL_IF_TRUE(!index.isValid(), QVariant());

    if (index.row() >= m_executeRecords.size() || index.column() >= EXECUTE_TABLE_COL)
    {
        KLOG_WARNING() << "The index exceeds range limit.";
        return QVariant();
    }

    auto trustedRecord = m_executeRecords[index.row()];

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

QVariant ExecuteProtectedModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    RETURN_VAL_IF_TRUE(orientation == Qt::Orientation::Vertical, QVariant())

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
            return "";
        case ExecuteField::EXECUTE_FIELD_STATUS:
            return "";
        default:
            break;
        }
    }
    default:
        break;
    }
    return QVariant();
}

bool ExecuteProtectedModel::setData(const QModelIndex &index,
                                    const QVariant &value,
                                    int role)
{
    RETURN_VAL_IF_TRUE(index.column() != 0, false)

    m_executeRecords[index.row()].selected = value.toBool();
    emit dataChanged(index, index);

    if (role == Qt::UserRole || role == Qt::EditRole)
    {
        checkSelectStatus();
    }

    return true;
}

Qt::ItemFlags ExecuteProtectedModel::flags(const QModelIndex &index) const
{
    RETURN_VAL_IF_TRUE(index.column() == 0, Qt::ItemFlag::ItemIsEnabled)

    return Qt::ItemFlag::NoItemFlags;
}

void ExecuteProtectedModel::updateRecord()
{
    beginResetModel();
    SCOPE_EXIT(
        {
            endResetModel();
        });

    m_executeRecords.clear();
    // 刷新时checkbox状态清空
    emit stateChanged(Qt::Unchecked);

    auto reply = m_tpDBusProxy->GetTrustedFiles(SSRKSSTrustedFileType::SSR_KSS_TRUSTED_FILE_TYPE_EXECUTE);
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
                                        .md5 = kssData.value(KSS_JSON_KEY_DATA_HASH).toString()};
        m_executeRecords.push_back(fileRecord);
    }
    emit filesUpdate(m_executeRecords.size());  // NOSONAR
}

QList<TrustedRecord> ExecuteProtectedModel::getExecuteRecords()
{
    return m_executeRecords;
}

void ExecuteProtectedModel::checkSelectStatus()
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

    emit stateChanged(state);
}

ExecuteProtectedTable::ExecuteProtectedTable(QWidget *parent)
    : QTableView(parent),
      m_filterProxy(nullptr)
{
    initTable();
    initTableHeaderButton();
}

void ExecuteProtectedTable::setSearchText(const QString &text)
{
    m_searchText = text;
    m_filterProxy->setSearchText(m_searchText);
    filterFixedString();
}

void ExecuteProtectedTable::updateInfo()
{
    m_model->updateRecord();
}

QList<TrustedRecord> ExecuteProtectedTable::getExecuteRecords()
{
    return m_model->getExecuteRecords();
}

int ExecuteProtectedTable::getExecutetamperedNums()
{
    int executetamperedNums = 0;
    for (auto executeRecord : m_model->getExecuteRecords())
    {
        if (executeRecord.status != tr("Certified"))
        {
            executetamperedNums++;
        }
    }
    return executetamperedNums;
}

void ExecuteProtectedTable::showDetails(const QModelIndex &index)
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
}

void KS::TP::ExecuteProtectedTable::initTable()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置Model
    m_model = new ExecuteProtectedModel(this);
    m_headerViewProxy = new TableHeaderProxy(this);
    setHorizontalHeader(m_headerViewProxy);
    setMouseTracking(true);

    connect(m_headerViewProxy, &TableHeaderProxy::toggled, this, &ExecuteProtectedTable::checkedAllItem);
    connect(m_model, &ExecuteProtectedModel::stateChanged, m_headerViewProxy, &TableHeaderProxy::setCheckState);
    connect(m_model, &ExecuteProtectedModel::filesUpdate, this, &ExecuteProtectedTable::filesUpdate);

    m_filterProxy = new ExecuteProtectedFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);

    setShowGrid(false);
    // 设置Delegate
    setItemDelegate(new Delegate(this));

    // 设置水平行表头
    m_headerViewProxy->resizeSection(ExecuteField::EXECUTE_FIELD_CHECKBOX, 50);
    m_headerViewProxy->resizeSection(ExecuteField::EXECUTE_FIELD_NUMBER, 100);
    m_headerViewProxy->resizeSection(ExecuteField::EXECUTE_FIELD_FILE_PATH, 350);
    m_headerViewProxy->resizeSection(ExecuteField::EXECUTE_FIELD_FILE_TYPE, 100);
    m_headerViewProxy->resizeSection(ExecuteField::EXECUTE_FIELD_STATUS, 100);

    m_headerViewProxy->setStretchLastSection(true);
    m_headerViewProxy->setSectionsMovable(false);
    m_headerViewProxy->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_headerViewProxy->setFixedHeight(24);

    // 设置垂直列表头
    auto verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);

    connect(this, &ExecuteProtectedTable::entered, this, &ExecuteProtectedTable::showDetails);
    connect(this, &ExecuteProtectedTable::clicked, this, &ExecuteProtectedTable::showDetails);
}

void ExecuteProtectedTable::initTableHeaderButton()
{
    // 文件类型筛选
    m_fileTypeButton = new HeaderButtonDelegate(this);
    m_fileTypeButton->setButtonText(tr("Type"));

    auto executeFiles = new QAction(tr("Executable file"), m_fileTypeButton);
    auto executeScripts = new QAction(tr("Executable script"), m_fileTypeButton);
    auto dynamicLibrary = new QAction(tr("Dynamic library"), m_fileTypeButton);
    m_fileTypeKeys << tr("Executable file") << tr("Executable script") << tr("Dynamic library");
    m_filterMap.insert("fileTypeButton", m_fileTypeKeys);
    m_fileTypeButton->addMenuActions(QList<QAction *>() << executeFiles << executeScripts << dynamicLibrary);
    connect(m_fileTypeButton, &HeaderButtonDelegate::menuTriggered, this, [this]()
            {
                for (auto action : m_fileTypeButton->getMenuActions())
                {
                    if (action->isChecked())
                    {
                        m_fileTypeKeys << action->text();
                    }
                    else
                    {
                        m_fileTypeKeys.removeAll(action->text());
                    }
                    // 去重
                    m_fileTypeKeys = QSet<QString>::fromList(m_fileTypeKeys).toList();

                    m_filterMap.insert("fileTypeButton", m_fileTypeKeys);
                }
                filterFixedString();
            });
    // 状态筛选
    m_statusButton = new HeaderButtonDelegate(this);
    m_statusButton->setButtonText(tr("Status"));

    auto certified = new QAction(tr("Certified"), m_statusButton);
    auto beingTamperedWith = new QAction(tr("Being tampered with"), m_statusButton);
    m_statusKeys << tr("Certified") << tr("Being tampered with");
    m_filterMap.insert("statusButton", m_statusKeys);
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
                    m_filterMap.insert("statusButton", m_statusKeys);
                }
                filterFixedString();
            });
    QMap<int, HeaderButtonDelegate *> headerButtons;
    headerButtons.insert(EXECUTE_FIELD_FILE_TYPE, m_fileTypeButton);
    headerButtons.insert(EXECUTE_FIELD_STATUS, m_statusButton);
    m_headerViewProxy->setHeaderButtons(headerButtons);
    filterFixedString();
}

void KS::TP::ExecuteProtectedTable::filterFixedString()
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

void ExecuteProtectedTable::checkedAllItem(Qt::CheckState checkState)
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
