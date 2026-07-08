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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
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
namespace BR
{
class Configuration : public QObject
{
    Q_OBJECT
public:
    Configuration(const QString& configPath);
    virtual ~Configuration();

    static Configuration* getInstance()
    {
        return m_instance;
    };

    static void globalInit(const QString& configPath);

    static void globalDeinit()
    {
        delete m_instance;
    };

    // 获取最大线程数
    uint32_t getMaxThreadNum();
    // 获取标准类型
    BRStandardType getStandardType();
    // 设置标准类型
    bool setStandardType(BRStandardType standardType);
    // 获取加固策略类型
    BRStrategyType getStrategyType();
    // 设置加固策略类型
    bool setStrategyType(BRStrategyType strategyType);
    // 获取自定义加固参数，只有在自定义策略模式下有效，系统策略返回空值
    QSharedPointer<Protocol::RA> getCustomRA();
    // 设置加固参数
    bool setCustomRA(const Protocol::Reinforcement& reinforcement);
    // 删除加固项的自定义参数
    void delCustomRA(const QString& name);
    void delAllCustomRA();
    // 获取定时扫描时间
    int getTimeScan();
    // 设置定时扫描时间
    bool setTimeScan(int time_scan);
    // 获取通知状态
    BRNotificationStatus getNotificationStatus();
    // 设置通知状态
    bool setNotificationStatus(BRNotificationStatus notification_status);
    // 获取回退状态
    BRFallbackStatus getFallbackStatus();
    // 设置回退状态
    bool setFallbackStatus(BRFallbackStatus fallbackStatus);
    // 检测导入ra文件是否正确
    bool checkRaStrategy();
    // 前台复选框勾选调用，checkbox后台默认为false
    void setRaCheckbox(const QString& name, const bool& status);
    // 获取加固标准
    QSharedPointer<Protocol::RS> getRS();
    // 设置自定义加固标准
    bool setCustomRS(const QString& encrypted_rs, SSRErrorCode& error_code);
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

    // 修改加固参数，重载加固项
    void reloadStrategy();
    // 加载加固标准文件(不变化的部分)
    QSharedPointer<Protocol::RS> getFixedRS();
    // 加载加固参数文件
    QSharedPointer<Protocol::RA> readRaFromFile();
    // 写加固参数文件
    bool writeRAToFile(QSharedPointer<Protocol::RA> ra);

    void joinReinforcement(Protocol::Reinforcement& to_r, const Protocol::Reinforcement& from_r);

    // 解密文件并返回字符串
    QString decryptFile(const QString& filename);

    int32_t getInteger(const QString& group_name, const QString& key, int32_t default_value = 0);
    QString getString(const QString& group_name, const QString& key);
    // 通过group_name和key获取basename，然后返回${datadir}/basename
    QString getDatadirFilename(const QString& group_name, const QString& key);
    void setInteger(const QString& group_name, const QString& key, int32_t value);
    void setString(const QString& group_name, const QString& key, const QString& value);

Q_SIGNALS:
    // 加固标准发生变化
    void RSChanged();
    // 加固策略发生变化
    void StrategyChanged();
    // 自定义加固参数变化
    void customRAChanged();

private:
    static Configuration* m_instance;

    // 配置文件路径
    QString m_configPath;
    // 配置文件内容
    QSettings* m_settings;
};
}  // namespace BR
}  // namespace KS
