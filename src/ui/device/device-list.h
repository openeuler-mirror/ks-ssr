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

#include <QWidget>

namespace Ui
{
class DeviceList;
}

namespace KS
{
class DevicePermission;
class DeviceList : public QWidget
{
    Q_OBJECT

public:
    DeviceList(QWidget *parent = nullptr);
    virtual ~DeviceList();

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void initTableStyle();

private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void popupEditDialog(const QModelIndex &index);
    void updateDevice();
    void updateCursor(const QModelIndex &index);

private:
    Ui::DeviceList *m_ui;
    DevicePermission *m_devicePermission;
};
}  //namespace KS
