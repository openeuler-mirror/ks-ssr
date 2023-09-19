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

namespace Ui {
class EditPermissions;
}

namespace KS {

enum PMDeviceStatus
{
    PM_DEVICE_STATUS_ENABLE,     //启用
    PM_DEVICE_STATUS_DISABLE,    //禁用
    PM_DEVICE_STATUS_UNACTIVE    //未授权
};

enum PMPermissionsType
{
    PM_PERMISSIONS_TYPE_READ,
    PM_PERMISSIONS_TYPE_WRITE,
    PM_PERMISSIONS_TYPE_EXEC
};

class EditPermissions : public QWidget
{
    Q_OBJECT

public:
    EditPermissions(const QString name, const PMDeviceStatus status, const QList<PMPermissionsType> permission, QWidget *parent = nullptr);
    ~EditPermissions();

protected:
    void paintEvent(QPaintEvent *event);

private:
    void initComboBox();
    void initBtns();

private slots:
    void onConfirm();
    void onStatusChanged(int index);

signals:
    void permissionChanged(int status, QList<PMPermissionsType> permissions);

private:
    Ui::EditPermissions *m_ui;
    QString m_name;
    PMDeviceStatus m_status;
    QList<PMPermissionsType> m_permission;
};
}   //namespace KS
