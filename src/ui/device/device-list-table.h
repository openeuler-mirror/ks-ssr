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

class ListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ListDelegate(QObject *parent = 0);
    virtual ~ListDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

//class DeviceListModel : public QStandardItemModel
//{
//    Q_OBJECT

//public:
//    DeviceListModel(QObject *parent = nullptr);
//    virtual ~DeviceListModel(){};

//    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
//    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

//    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
//    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

//    //bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
//    //void setData(QList<DeviceInfo> deviceInfos);

//private:
//    //TableFilterModel *m_DeviceProxy;
//    QList<DeviceInfo> m_deviceInfo;
//};

class TableFilterModel;
class DeviceListTable : public QTableView
{
    Q_OBJECT
public:
    DeviceListTable(QWidget *parent = nullptr);
    void setData(const QList<DeviceInfo> &infos);
    int getColCount();
    int getRowCount();
    TableFilterModel *getFilterProxy();

private:
    void setHeaderSections(QStringList sections);

private:
    TableFilterModel *m_filterProxy;
    QStandardItemModel *m_model;
    ListDelegate *m_delegate;
};
}  // namespace KS
