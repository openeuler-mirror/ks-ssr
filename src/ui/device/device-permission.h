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
#include "include/ksc-i.h"
#include "src/ui/common/titlebar-window.h"
namespace Ui
{
class DevicePermission;
}

namespace KS
{
class DevicePermission : public TitlebarWindow
{
    Q_OBJECT

public:
    DevicePermission(const QString &name, const QString &id, QWidget *parent = nullptr);
    virtual ~DevicePermission();
    QString getDeviceID();
    void setDeviceStatus(const DeviceState &status);
    void setDevicePermission(int permission);
    DeviceState getDeviceStatus();
    int getDevicePermission();

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void confirm();
    void updateGroupBox(int index);

signals:
    void permissionChanged();

private:
    Ui::DevicePermission *m_ui;
    QString m_name;
    QString m_id;
    DeviceState m_status;
    int m_permissions;
};
}  //namespace KS
