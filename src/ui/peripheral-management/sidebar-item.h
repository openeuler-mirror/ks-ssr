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

namespace Ui {
class SidebarItem;
}
namespace KS
{
class SidebarItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectChanged)

public:
    explicit SidebarItem(const QString text, const QString icon, QWidget *parent = nullptr);
    ~SidebarItem();

public:
    void setSelected(bool isSelected);
    bool isSelected();

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::SidebarItem *m_ui;
    bool m_isSelected;

signals:
    void selectChanged(bool isSelected);
};
}   //namespace ks

