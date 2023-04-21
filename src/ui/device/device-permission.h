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
#include "src/ui/common/titlebar-window.h"

namespace Ui
{
class DevicePermission;
}

namespace KS
{
/**
 * @brief 设备状态
 */
enum DeviceStatus
{
    //启用
    DEVICE_STATUS_ENABLE,
    //禁用
    DEVICE_STATUS_DISABLE,
    //未授权
    DEVICE_STATUS_UNACTIVE,
    DEVICE_STATUS_LAST
};

/**
 * @brief 设备权限
 */
enum DevicePermissionType
{
    DEVICE_PERMISSION_TYPE_READ = (1 << 0),
    DEVICE_PERMISSION_TYPE_WRITE = (1 << 1),
    DEVICE_PERMISSION_TYPE_EXEC = (1 << 2)
};

class DevicePermission : public QWidget
{
    Q_OBJECT

public:
    DevicePermission(const QString &name, QWidget *parent = nullptr);
    virtual ~DevicePermission();
    void setDeviceStatus(const DeviceStatus &status);
    void setDevicePermission(int permission);
    DeviceStatus getDeviceStatus();
    int getDevicePermission();

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void confirm();
    void updateGroupBox(int index);

signals:
    void permissionChanged();

private:
    Ui::DevicePermission *m_ui;
    QString m_name;
    DeviceStatus m_status;
    int m_permissions;
};
}  //namespace KS
