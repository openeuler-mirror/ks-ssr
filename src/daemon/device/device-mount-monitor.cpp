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

#include "src/daemon/device/device-mount-monitor.h"
#include <qt5-log-i.h>
#include <QFile>
#include "config.h"
#include "ksc-marcos.h"

namespace KS
{
DeviceMountMonitor::DeviceMountMonitor(QObject *parent) : QObject(parent)
{
    this->initMounts();
    this->initWatcher();
}

DeviceMountMonitor::~DeviceMountMonitor()
{
    timer.stop();
}

void DeviceMountMonitor::initMounts()
{
    m_mounts = this->processMountFile();
}

void DeviceMountMonitor::check()
{
    auto mounts = this->processMountFile();
    Q_FOREACH (auto mount, mounts)
    {
        this->checkMount(mount);
    }

    m_mounts = mounts;
}

void DeviceMountMonitor::initWatcher()
{
    timer.setTimerType(Qt::PreciseTimer);
    QObject::connect(&timer, &QTimer::timeout, this, &DeviceMountMonitor::check);
    timer.start(1000);
}

QMap<QString, QSharedPointer<DeviceMount>> DeviceMountMonitor::processMountFile()
{
    QMap<QString, QSharedPointer<DeviceMount>> mounts;
    QFile file(KSC_DEVICE_SYS_MOUNT_FILE);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "Cannot open file " << KSC_DEVICE_SYS_MOUNT_FILE;
        return mounts;
    }

    QByteArray contents = file.readAll();
    QTextStream in(&contents);

    while (!in.atEnd())
    {
        QString line = in.readLine();

        auto mount = this->processMountLine(line);

        if (mount)
        {
            mounts.insert(mount->device, mount);
        }
    }

    return mounts;
}

QSharedPointer<DeviceMount> DeviceMountMonitor::processMountLine(const QString mountLine)
{
    RETURN_VAL_IF_FALSE(mountLine.startsWith("/dev"), nullptr)
    auto strs = mountLine.split(" ");
    auto device = strs.at(0);
    auto path = strs.at(1);
    auto permissions = strs.at(3);

    return QSharedPointer<DeviceMount>(new DeviceMount{
        .device = device,
        .path = path,
        .read = true,
        .write = permissions.startsWith("rw,"),
        .execute = !permissions.contains(",noexec,"),
    });
}

DeviceMountList DeviceMountMonitor::getMounts()
{
    DeviceMountList mounts;
    for (auto mount : m_mounts)
    {
        mounts.push_back(mount);
    }
    return mounts;
}

void DeviceMountMonitor::checkMount(const QSharedPointer<DeviceMount> mount)
{
    auto prevMount = m_mounts.value(mount->device);

    if (!prevMount ||
        (prevMount->write != mount->write) ||
        (prevMount->execute != mount->execute))
    {
        //设备挂载发生变化
        Q_EMIT this->mountChanged(mount.get());
    }
}
}  // namespace KS
