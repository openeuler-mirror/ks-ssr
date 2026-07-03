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

#pragma once

#include <QAbstractTableModel>
#include <QList>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QWidget>
#include "src/ui/common/table/table-header-proxy.h"
#include "ssr-i.h"

class LogProxy;

namespace KS
{
class HeaderButtonDelegate;

namespace Log
{
// 一页多少条日志
#define LOG_PAGE_NUMBER 20

enum LogType
{
    // 设备日志
    LOG_TYPE_DEVICE = (1 << 0),
    // 工具箱日志
    LOG_TYPE_TOOL_BOX = (1 << 1),
    // 基线加固
    LOG_TYPE_BASELINE_REINFORCEMENT = (1 << 2),
    // 可信保护
    LOG_TYPE_TRUSTED_PROTECTION = (1 << 3),
    // 文件保护
    LOG_TYPE_FILES_PROTECTION = (1 << 4),
    // 保险箱
    LOG_TYPE_PRIVATE_BOX = (1 << 5),
    // 账户日志
    LOG_TYPE_ACCOUNT = (1 << 6),
    LOG_TYPE_AVC = (1 << 7)
};

struct LogInfo
{
    // 日志类型
    LogType type;
    // 应用资源 不理解标注图上这一列的作用，是否与日志类型是一个含义
    //  QString resource;
    AccountRole role;
    // 日期时间
    QString dataTime;
    // 操作
    QString message;
    // 结果
    bool result;
};

class LogDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    LogDelegate(QObject *parent = 0);
    virtual ~LogDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class LogFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    LogFilterModel(QObject *parent = nullptr);
    virtual ~LogFilterModel(){};

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class LogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    struct GetLogArgs
    {
        uint role;
        qlonglong timeStampBegin;
        qlonglong timeStampEnd;
        LogType type;
        uint result;
        uint currentPage;
        QString searchKey;
    };

    LogModel(QObject *parent = nullptr);
    virtual ~LogModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    uint getLogNumbers();

    // 改变索引
    // 设置权限分类
    void setRole(uint role);
    void setTimeStampBegin(qlonglong timeStampBegin);
    void setTimeStampEnd(qlonglong timeStampEnd);
    void setLogType(LogType type);
    void setLogResult(uint result);
    void setCurrentPage(uint currentPage);
    void setSearchKey(const QString &text);

private:
    void initGetLogArgs();
    void updateRecord();

signals:
    void logUpdated(int total);

private:
    LogProxy *m_logProxy;
    QList<LogInfo> m_logInfos;
    GetLogArgs m_args;
};

class LogTable : public QTableView
{
    Q_OBJECT

public:
    LogTable(QWidget *parent = nullptr);
    virtual ~LogTable(){};

    void search(const QString &text);
    uint getLogNumbers();
    void setCurrentPage(uint currentPage);
    void setTimeStampBegin(qlonglong timeStampBegin);
    void setTimeStampEnd(qlonglong timeStampEnd);

private:
    void initTable();
    void initTableHeaderButton();
    void initLogTypeButton();
    void initRoleButton();
    void initResultButton();

private slots:
    void mouseEnter(const QModelIndex &index);

signals:
    void logUpdated(int total);

private:
    LogFilterModel *m_filterProxy;
    LogModel *m_model;
    TableHeaderProxy *m_headerViewProxy;

    HeaderButtonDelegate *m_logTypeButton;
    HeaderButtonDelegate *m_roleButton;
    HeaderButtonDelegate *m_resultButton;
};
}  // namespace Log
}  // namespace KS
