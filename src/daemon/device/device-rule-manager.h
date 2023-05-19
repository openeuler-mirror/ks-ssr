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
// TODO: 这里应该也不是DeviceRule，只有UDEV才叫Rule，这里只能说是控制参数
struct DeviceRule
{
public:
    DeviceRule() = default;
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

// TODO: 类名还要想一下，应该叫DeviceConfiguration好一些，如果换名字的话，类里面其他名字也要换
class DeviceRuleManager : public QObject
{
    Q_OBJECT

public:
    static DeviceRuleManager *instance();
    void addRule(const DeviceRule &rule);
    QSharedPointer<DeviceRule> getRule(const QString &uid);

    bool isIFCEnable(int type);
    void setIFCEnable(int type, bool enable);

private:
    explicit DeviceRuleManager(QObject *parent = nullptr);

private:
    void init();

    // 将设备控制文件同步到对应的系统配置中
    void syncDeviceFile();
    // 将设备控制配置同步到udev规则文件
    void syncToDeviceUdevFile();
    // 设备规则对象转udev字符串
    QString ruleObj2Str(QSharedPointer<DeviceRule> rule);
    QString getUdevModeValue(QSharedPointer<DeviceRule> rule);

    // 将接口控制文件同步到对应的系统配置中
    void syncInterfaceFile();
    // 将接口控制配置同步到udev规则文件
    void syncToInterfaceUdevFile();
    // 根据接口类型获取udev规则
    QString getInterfaceUdevRule(int type);
    // 将接口控制配置同步到grub文件
    void syncInterfaceToGrubFile();
    // 获取系统中所有HDMI接口名称
    QStringList getHDMINames();
    // 同步更新蓝牙服务
    void syncToBluetoothService();

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
