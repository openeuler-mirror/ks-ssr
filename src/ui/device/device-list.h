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

#include <QWidget>
#include "src/ui/device/device-list-table.h"
#include "src/ui/device_manager_proxy.h"

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
    void update();

protected:
    void paintEvent(QPaintEvent *event);

private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void popupEditDialog(const QModelIndex &index);
    void updatePermission();
    void updateState();

private:
    Ui::DeviceList *m_ui;
    DevicePermission *m_devicePermission;
    DeviceManagerProxy *m_deviceManagerProxy;
};
}  //namespace KS
