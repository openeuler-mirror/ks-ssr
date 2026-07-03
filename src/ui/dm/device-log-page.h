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

namespace Ui
{
class DeviceLogPage;
}

namespace KS
{
namespace DM
{
class DeviceLogPage : public Page
{
    Q_OBJECT

public:
    DeviceLogPage(QWidget *parent = nullptr);
    virtual ~DeviceLogPage();
    void update();

    QString getNavigationUID() override;
    QString getSidebarUID() override;
    QString getSidebarIcon() override;
    QString getAccountRoleName() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private Q_SLOTS:
    void searchTextChanged(const QString &text);

private:
    Ui::DeviceLogPage *m_ui;
    DeviceManagerProxy *m_deviceManagerProxy;
};
}  // namespace DM
}  // namespace KS
