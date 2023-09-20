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
#include "include/ksc-i.h"
#include "src/ui/device_manager_proxy.h"

namespace KS
{
struct DeviceInfo
{
    DeviceInfo() = default;
    int number;
    QString name;
    DeviceType type;
    QString id;
    InterfaceType interface;
    DeviceState state;
    int permission;
};

enum ListTableField
{
    LIST_TABLE_FIELD_NUMBER,
    LIST_TABLE_FIELD_NAME,
    LIST_TABLE_FIELD_TYPE,
    LIST_TABLE_FIELD_ID,
    LIST_TABLE_FIELD_INTERFACE,
    LIST_TABLE_FIELD_STATUS,
    LIST_TABLE_FIELD_PERMISSION,
    LIST_TABLE_FIELD_LAST
};

class DeviceListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DeviceListDelegate(QObject *parent = 0);
    virtual ~DeviceListDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class TableFilterModel;
class DeviceListTable : public QTableView
{
    Q_OBJECT
public:
    DeviceListTable(QWidget *parent = nullptr);

    void update();
    void setData(const QList<DeviceInfo> &infos);
    DeviceState getState(int row);
    QString getID(int row);
    QString getName(int row);
    int getPermission(int row);
    int getColCount();
    int getRowCount();
    TableFilterModel *getFilterProxy();

protected:
    void leaveEvent(QEvent *event);

private:
    void initTable();
    void setHeaderSections(QStringList sections);

private slots:
    void updateCusor(const QModelIndex &index);
    void showDetails(const QModelIndex &index);

private:
    TableFilterModel *m_filterProxy;
    QStandardItemModel *m_model;
    DeviceManagerProxy *m_deviceManagerProxy;
    QList<DeviceInfo> m_devicesInfo;
};
}  // namespace KS