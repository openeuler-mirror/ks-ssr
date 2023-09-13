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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#pragma once

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QStringBuilder>
#include "br-protocol.hxx"
#include "lib/base/base.h"

namespace KS
{
namespace BRDaemon
{
class Configuration : public QObject
{
    Q_OBJECT
public:
    Configuration(const QString& config_path);
    virtual ~Configuration();

    static Configuration* getInstance() { return instance_; };

    static void globalInit(const QString& config_path);

    static void globalDeinit() { delete instance_; };

    // 获取最大线程数
    uint32_t getMaxThreadNum();
    // 获取标准类型
    BRStandardType getStandardType();
    // 设置标准类型
    bool setStandardType(BRStandardType standard_type);
    // 获取加固策略类型
    BRStrategyType getStrategyType();
    // 设置加固策略类型
    bool setStrategyType(BRStrategyType strategy_type);
    // 获取定时扫描时间
    int getTimeScan();
    // 设置定时扫描时间
    bool setTimeScan(int time_scan);
    // 获取通知状态
    BRNotificationStatus getNotificationStatus();
    // 设置通知状态
    bool setNotificationStatus(BRNotificationStatus notification_status);
    // 检测导入ra文件是否正确
    bool checkRaStrategy();
    // 前台复选框勾选调用，checkbox后台默认为false
    void setRaCheckbox(const QString& name, const bool& status);
    // 获取加固标准
    QSharedPointer<Protocol::RS> getRs() { return this->rs_; }
    // 设置自定义加固标准
    bool setCustomRs(const QString& encrypted_rs, BRErrorCode& error_code);
    // 设置加固参数
    bool setCustomRa(const Protocol::Reinforcement& rs_reinforcement);
    // 删除加固项的自定义参数
    void delCustomRa(const QString& name);
    void delAllCustomRa();

    // 加载历史加固参数文件
    std::shared_ptr<Protocol::ReinforcementHistory> readRhFromFile(const QString path);
    // 写历史加固参数文件
    bool writeRhToFile(std::shared_ptr<Protocol::ReinforcementHistory> rh, const QString path);
    // 设置历史加固参数
    bool setCustomRh(const Protocol::Reinforcement& rs_reinforcement, const QString path);

    BRResourceMonitor getResourceMonitorStatus();
    bool setResourceMonitorStatus(BRResourceMonitor resource_monitor);

private:
    //
    void init();

    // 重新加载加固标准，这里会发送变化的信号
    void reloadRs();
    void loadRs();
    // 修改加固参数，重载加固项
    void reloadStrategy();
    // 加载加固标准文件(不变化的部分)
    QSharedPointer<Protocol::RS> getFixedRs();
    // 加载加固参数文件
    QSharedPointer<Protocol::RA> readRaFromFile();
    // 写加固参数文件
    bool writeRaToFile(QSharedPointer<Protocol::RA> ra);

    void joinReinforcement(Protocol::Reinforcement& to_r, const Protocol::Reinforcement& from_r);

    // 解密文件并返回字符串
    QString decryptFile(const QString& filename);

    int32_t getInteger(const QString& group_name, const QString& key, int32_t default_value = 0);
    QString getString(const QString& group_name, const QString& key);
    // 通过group_name和key获取basename，然后返回${datadir}/basename
    QString getDatadirFilename(const QString& group_name, const QString& key);
    void setInteger(const QString& group_name, const QString& key, int32_t value);
    void setString(const QString& group_name, const QString& key, const QString& value);

private:
    static Configuration* instance_;

    // 配置文件路径
    QString config_path_;
    // 配置文件内容
    QSettings* configuration_;

    // 加固标准和自定义加固参数的混合
    QSharedPointer<Protocol::RS> rs_;

    // sigc::signal<void> rs_changed_;
signals:
    void rs_changed_();
};
}  // namespace BRDaemon
}  // namespace KS
