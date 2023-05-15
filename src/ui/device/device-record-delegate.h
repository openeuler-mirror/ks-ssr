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

#include <QStyledItemDelegate>

namespace KS
{
enum RecordTableField
{
    RECORD_TABLE_FIELD_NAME,
    RECORD_TABLE_FIELD_TYPE,
    RECORD_TABLE_FIELD_TIME,
    RECORD_TABLE_FIELD_STATUS,
    RECORD_TABLE_FIELD_LAST
};

class DeviceRecordDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DeviceRecordDelegate(QObject *parent = 0);
    virtual ~DeviceRecordDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
}  // namespace KS
