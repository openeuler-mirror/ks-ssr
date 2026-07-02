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
struct RecordsInfo
{
    QString name;
    int type;
    QString time;
    int status;
};

class RecordDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    RecordDelegate(QObject *parent = 0);
    virtual ~RecordDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class TableFilterModel;
class DeviceRecordsTable : public QTableView
{
    Q_OBJECT
public:
    DeviceRecordsTable(QWidget *parent = nullptr);
    void setData(const QList<RecordsInfo> &infos);
    int getColCount();
    int getRowCount();
    TableFilterModel *getFilterProxy();

private:
    void setHeaderSections(QStringList sections);

private:
    TableFilterModel *m_filterProxy;
    QStandardItemModel *m_model;
    RecordDelegate *m_delegate;
};
}  // namespace KS
