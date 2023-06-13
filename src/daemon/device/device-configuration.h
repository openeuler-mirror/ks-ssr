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
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#pragma once

#include <QObject>
#include <QSettings>
#include <QSharedPointer>

class QThread;

namespace KS
{
struct DeviceConfig
{
public:
    DeviceConfig() = default;
    QString uid;
    QString id;
    QString name;
    QString idVendor;
    QString idProduct;
    bool read;
    bool write;
    bool execute;
    bool enable;
    int type;
    int interfaceType;
};

using DeviceConfigList = QList<QSharedPointer<DeviceConfig>>;

class DeviceConfiguration : public QObject
{
    Q_OBJECT

public:
    static DeviceConfiguration *instance();
    void addConfig(const DeviceConfig &Config);
    QSharedPointer<DeviceConfig> getDeviceConfig(const QString &uid);
    //获取所有的设备配置
    DeviceConfigList getDeviceConfig();

    bool isIFCEnable(int type);
    void setIFCEnable(int type, bool enable);

signals:
    void deviceConfigChanged();

private:
    explicit DeviceConfiguration(QObject *parent = nullptr);

private:
    void init();

    // 将接口控制文件同步到对应的系统配置中
    void syncInterfaceFile();
    // 将接口控制配置同步到grub文件
    void syncInterfaceToGrubFile();
    // 获取系统中所有HDMI接口名称
    QStringList getHDMINames();
    // 同步更新蓝牙服务
    void syncToBluetoothService();
    // 同步更新网络服务
    void syncToNMService();

    void saveToFile(const QStringList &lines, const QString &filename);
    // 在线程中更新相关的grub配置
    void updateGrubsInThread();
    // 检查是否有等待grub配置更新的命令
    void checkWaitingUpdateGrubs();
    // grub配置更新完毕
    void finishGrubsUpdate();
    // 更新单个grub配置
    void updateGrub(const QString &filePath);

private:
    // 设备控制相关配置
    QSettings *m_deviceSettings;
    // 接口控制相关配置
    QSettings *m_interfaceSettings;
    // grub更新线程
    QThread *m_grubUpdateThread;
    // 是否需要更新grub配置
    bool m_waitingUpdateGrub;
};
}  // namespace KS
