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

#include <QCheckBox>
#include <QGridLayout>
#include <QWidget>
#include "include/ksc-i.h"
#include "src/ui/device_manager_proxy.h"

struct Interface
{
    Interface() = default;
    InterfaceType type;
    bool enable;
};

namespace KS
{
class SettingsDevice : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsDevice(QWidget *parent = nullptr);
    ~SettingsDevice();

private:
    void initUI();
    void insertInterfaceWidget();
    void update();
    QList<Interface> getInterfaces();

private slots:
    void setInterfaceState(bool checked);

private:
    DeviceManagerProxy *m_deviceManagerProxy;
    QList<Interface> m_interfaces;
    QGridLayout *m_gridLayout;
    QGridLayout *m_usbLayout;
    QGridLayout *m_kbdMouseLayout;
    QWidget *m_kbdMouseContent;
    //绑定接口控制QCheckbox以及接口类型
    QMap<InterfaceType, QCheckBox *> m_checkboxs;
};
}  // namespace KS
