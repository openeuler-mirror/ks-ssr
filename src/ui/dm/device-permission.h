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
#include "include/ssr-i.h"
#include "src/ui/common/window/titlebar-window.h"

namespace Ui
{
class DevicePermission;
}

namespace KS
{
namespace DM
{
class DevicePermission : public TitlebarWindow
{
    Q_OBJECT

public:
    DevicePermission(QWidget *parent = nullptr);
    virtual ~DevicePermission();

    void setDeviceID(const QString &id);
    QString getDeviceID();

    void setDeviceStatus(const DeviceState &status);
    DeviceState getDeviceStatus();

    void setDevicePermission(const QString type, int permission);
    int getDevicePermission();

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void confirm();
    void update(int index);

signals:
    void permissionChanged();
    void stateChanged();
    void deviceChanged();

private:
    Ui::DevicePermission *m_ui;
    QString m_id;
    DeviceState m_status;
    int m_permissions;
};
}  // namespace DM
}  // namespace KS