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
enum PMDeviceStatus
{
    //启用
    PM_DEVICE_STATUS_ENABLE,
    //禁用
    PM_DEVICE_STATUS_DISABLE,
    //未授权
    PM_DEVICE_STATUS_UNACTIVE,
    PM_DEVICE_STATUS_LAST
};

/**
 * @brief 设备权限
 */
enum PMPermissionsType
{
    PM_PERMISSIONS_TYPE_READ,
    PM_PERMISSIONS_TYPE_WRITE,
    PM_PERMISSIONS_TYPE_EXEC
};

class DevicePermission : public QWidget
{
    Q_OBJECT

public:
    DevicePermission(const QString &name, QWidget *parent = nullptr);
    ~DevicePermission();
    void setDeviceStatus(PMDeviceStatus status);
    void setDevicePermission(QList<PMPermissionsType> permission);
    PMDeviceStatus getDeviceStatus();
    QList<PMPermissionsType> getDevicePermission();

protected:
    void paintEvent(QPaintEvent *event);

private:
    int enmu2int(PMDeviceStatus status);
    PMDeviceStatus int2enum(int status);

private slots:
    void onConfirm();
    void onStatusChanged(int index);

signals:
    void okCliecked();

private:
    Ui::DevicePermission *m_ui;
    QString m_name;
    PMDeviceStatus m_status;
    QList<PMPermissionsType> m_permissions;
};
}  //namespace KS
