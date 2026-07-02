/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
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

#include <QObject>
#include <QStyledItemDelegate>

namespace KS
{
enum DeviceTableField
{
    DEVICE_TABLE_FIELD_NUMBER,
    DEVICE_TABLE_FIELD_NAME,
    DEVICE_TABLE_FIELD_TYPE,
    DEVICE_TABLE_FIELD_ID,
    DEVICE_TABLE_FIELD_INTERFACE,
    DEVICE_TABLE_FIELD_STATUS,
    DEVICE_TABLE_FIELD_PERMISSION,
    DEVICE_TABLE_FIELD_LAST
};

class DeviceListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DeviceListDelegate(QObject *parent = 0);
    virtual ~DeviceListDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
}  // namespace KS
