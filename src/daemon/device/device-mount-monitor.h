/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVDescriptionED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#pragma once

#include <QMap>
#include <QObject>
#include <QTimer>

namespace KS
{
struct DeviceMount
{
public:
    DeviceMount() = default;
    //设备文件
    QString device;
    //设备挂载位置
    QString path;
    bool read;
    bool write;
    bool execute;
};

using DeviceMountList = QList<QSharedPointer<DeviceMount>>;

class DeviceMountMonitor : public QObject
{
    Q_OBJECT

public:
    DeviceMountMonitor(QObject *parent = nullptr);
    ~DeviceMountMonitor();

    DeviceMountList getMounts();

signals:
    void mountChanged(const DeviceMount *mount);

private slots:
    void check();

private:
    void initMounts();
    void initWatcher();
    QSharedPointer<DeviceMount> processMountLine(const QString mountLine);
    QMap<QString, QSharedPointer<DeviceMount>> processMountFile();
    void checkMount(const QSharedPointer<DeviceMount> mount);
    QMap<QString, QSharedPointer<DeviceMount>> m_mounts;
    QTimer timer;
};
}  // namespace KS
