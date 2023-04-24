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
#include <QWidget>

namespace KS
{
struct DeviceInfo
{
    int number;
    QString name;
    int type;
    QString id;
    int interface;
    int status;
    int permission;
};

struct RecordsInfo
{
    QString name;
    int type;
    QString time;
    int status;
};

class TableFilterModel;
class DeviceListDelegate;
class DeviceTable : public QTableView
{
    Q_OBJECT
public:
    DeviceTable(QWidget *parent = nullptr);
    void setHeaderSections(QStringList sections);
    void setDelegate(QAbstractItemDelegate *delegate);
    void setData(const QList<DeviceInfo> &infos);
    void setData(const QList<RecordsInfo> &infos);
    int getColCount();
    int getRowCount();
    TableFilterModel *getFilterProxy();

private:
    TableFilterModel *m_filterProxy;
    QStandardItemModel *m_model;
};
}  // namespace KS
