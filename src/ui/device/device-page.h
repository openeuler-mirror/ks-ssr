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
class DevicePage;
}

namespace KS
{
class SidebarItem;
class DeviceRecord;
class DeviceList;

class DevicePage : public QWidget
{
    Q_OBJECT

public:
    DevicePage(QWidget *parent = nullptr);
    virtual ~DevicePage();

protected:
    void paintEvent(QPaintEvent *event);

private:
    void initSidebar();
    void initSubPage();
    void createSideBarItem(const QString &text, const QString &icon);

private slots:
    void updatePage(int currentRow);

private:
    Ui::DevicePage *m_ui;
};
}  // namespace KS
