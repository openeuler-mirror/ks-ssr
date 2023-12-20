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

class LogProxy;

namespace KS
{
namespace Log
{
enum LogType
{
    // 设备日志
    LOG_TYPE_DEVICE,
    // 审计日志
    LOG_TYPE_AUDIT,
    // 可信日志
    LOG_TYPE_TRUSTED,
    LOG_TYPE_OTHER
};

struct LogInfo
{
    // 序号
    int number;
    // 日志类型
    LogType type;
    // 应用资源 不理解标注图上这一列的作用，是否与日志类型是一个含义
    //  QString resource;
    // 用户名
    QString userName;
    // 日期时间
    QString dataTime;
    // 操作
    QString message;
    // 结果
    QString result;
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
    LogModel(QObject *parent = nullptr);
    virtual ~LogModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QList<LogInfo> getLogInfos();

signals:
    void logUpdated(int total);

private:
    // TODO 需要修改函数逻辑，切换页面或搜索时调用更新
    void updateRecord();

private:
    LogProxy *m_logProxy;
    QList<LogInfo> m_logInfos;
};

class LogTable : public QTableView
{
    Q_OBJECT

public:
    LogTable(QWidget *parent = nullptr);
    virtual ~LogTable(){};

    LogFilterModel *getFilterProxy()
    {
        return m_filterProxy;
    };
    void searchTextChanged(const QString &text);
    QList<LogInfo> getLogInfos();

signals:
    void logUpdated(int total);

private:
    LogFilterModel *m_filterProxy;
    LogModel *m_model;
    TableHeaderProxy *m_headerViewProxy;
};
}  // namespace Log
}  // namespace KS
