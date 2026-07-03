/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-session-manager is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#pragma once

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include "src/ui/device_manager_proxy.h"

namespace KS
{
struct RecordInfo
{
    RecordInfo() = default;
    QString name;
    int type;
    QString time;
    int status;
};

enum LogTableField
{
    LOG_TABLE_FIELD_NAME,
    LOG_TABLE_FIELD_TYPE,
    LOG_TABLE_FIELD_TIME,
    LOG_TABLE_FIELD_STATUS,
    LOG_TABLE_FIELD_LAST
};

class DeviceLogDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DeviceLogDelegate(QObject *parent = 0);
    virtual ~DeviceLogDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class TableFilterModel;
class DeviceLogTable : public QTableView
{
    Q_OBJECT
public:
    DeviceLogTable(QWidget *parent = nullptr);
    void update();
    void setData(const QList<RecordInfo> &infos);
    int getColCount();
    int getRowCount();
    TableFilterModel *getFilterProxy();

private:
    void initTable();
    void setHeaderSections(QStringList sections);

private:
    TableFilterModel *m_filterProxy;
    QStandardItemModel *m_model;
    DeviceManagerProxy *m_deviceManagerProxy;
    QList<RecordInfo> m_recordsInfo;
};
}  // namespace KS
