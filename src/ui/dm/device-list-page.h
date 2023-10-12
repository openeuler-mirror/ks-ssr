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
#include "src/ui/common/page.h"
#include "src/ui/device_manager_proxy.h"
#include "src/ui/dm/device-list-table.h"

namespace Ui
{
class DeviceListPage;
}

namespace KS
{
namespace DM
{
class DevicePermission;
class DeviceListPage : public Page
{
    Q_OBJECT

public:
    DeviceListPage(QWidget *parent = nullptr);
    virtual ~DeviceListPage();
    void update();

    QString getNavigationUID() override;
    QString getSidebarUID() override;
    QString getSidebarIcon() override;
    int getSelinuxType() override;

protected:
    void paintEvent(QPaintEvent *event);

private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void popupEditDialog(const QModelIndex &index);
    void updatePermission();
    void updateState();

private:
    Ui::DeviceListPage *m_ui;
    DevicePermission *m_devicePermission;
    DeviceManagerProxy *m_deviceManagerProxy;
};
}  // namespace DM
}  // namespace KS
