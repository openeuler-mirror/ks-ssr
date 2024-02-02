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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#pragma once

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include "include/ssr-i.h"
#include "src/ui/device_manager_proxy.h"

namespace KS
{
class TableHeaderProxy;
class HeaderButtonDelegate;

namespace DM
{
struct DeviceInfo
{
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
    LIST_TABLE_FIELD_NUMBER = 0,
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
    QString getType(int row);
    QString getID(int row);
    QString getName(int row);
    int getPermission(int row);
    int getColCount();
    int getRowCount();
    void setSearchText(const QString &text);

protected:
    void leaveEvent(QEvent *event);

private:
    void initTable();
    void setHeaderSections(QStringList sections);
    void initTableHeaderButton();

    void filterFixedString();

private slots:
    void updateCusor(const QModelIndex &index);
    void showDetails(const QModelIndex &index);

private:
    TableFilterModel *m_filterProxy;
    QStandardItemModel *m_model;
    DeviceManagerProxy *m_deviceManagerProxy;
    TableHeaderProxy *m_headerViewProxy;
    HeaderButtonDelegate *m_deviceTypeButton;
    QStringList m_deviceTypeKeys;
    HeaderButtonDelegate *m_statusButton;
    QStringList m_statusKeys;
    QString m_searchText;
    QMap<QString, QStringList> m_filterMap;

    QList<DeviceInfo> m_devicesInfo;
};
}  // namespace DM
}  // namespace KS
