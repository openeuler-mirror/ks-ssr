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

#include "configuration.h"
#include <fstream>

namespace KS
{
namespace BR
{
#define BR_GROUP_NAME "br"
#define BR_BASE_KEY_MAX_THREAD_NUM "max_thread_num"
#define BR_BASE_KEY_STANDARD_TYPE "standard_type"
#define BR_BASE_KEY_STRATEGY_TYPE "strategy_type"
#define BR_BASE_KEY_RESOURCE_MONITOR "resource_monitor"
#define BR_BASE_KEY_TIME_SCAN "time_scan"
#define BR_BASE_KEY_NOTIFICATION_STATUS "notification_status"

#define MAX_THREAD_NUM_DEFAULT 1

#define SYSTEM_RS_FILEPATH SSR_INSTALL_DATADIR "/br-system-rs"
#define CUSTOM_RS_FILEPATH SSR_INSTALL_DATADIR "/br-custom-rs"

#define CUSTOM_RA_FILEPATH SSR_INSTALL_DATADIR "/br-custom-ra.xml"
#define CUSTOM_RA_STRATEGY_FILEPATH SSR_INSTALL_DATADIR "/br-custom-ra-strategy.xml"
#define RH_BR_DATDIR SSR_INSTALL_DATADIR "/ReinforcementHistory"
#define RSA_PUBLIC_KEY_FILEPATH SSR_INSTALL_DATADIR "/br-public.key"

using namespace Protocol;

Configuration::Configuration(const QString& configPath, QObject* parent)
    : QObject(parent),
      m_configPath(configPath),
      m_settings(nullptr)
{
}

Configuration::~Configuration()
{
    delete this->m_settings;
}

void Configuration::init()
{
    this->m_settings = new QSettings(this->m_configPath, QSettings::NativeFormat);
}

uint32_t Configuration::getMaxThreadNum()
{
    return this->getInteger(BR_GROUP_NAME, BR_BASE_KEY_MAX_THREAD_NUM, MAX_THREAD_NUM_DEFAULT);
}

BRStandardType Configuration::getStandardType()
{
    auto retval = this->getInteger(BR_GROUP_NAME,
                                   BR_BASE_KEY_STANDARD_TYPE,
                                   BRStandardType::BR_STANDARD_TYPE_SYSTEM);

    if (retval >= BRStandardType::BR_STANDARD_TYPE_LAST || retval < 0)
    {
        KLOG_WARNING("The standard type is invalid. standard type: %d.", retval);
        return BRStandardType::BR_STANDARD_TYPE_SYSTEM;
    }

    return BRStandardType(retval);
}

bool Configuration::setStandardType(BRStandardType standardType)
{
    RETURN_VAL_IF_FALSE(standardType < BRStandardType::BR_STANDARD_TYPE_LAST, false);
    RETURN_VAL_IF_TRUE(standardType == this->getStandardType(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_STANDARD_TYPE, int32_t(standardType));
    Q_EMIT RSChanged();
    return true;
}

BRStrategyType Configuration::getStrategyType()
{
    auto retval = this->getInteger(BR_GROUP_NAME,
                                   BR_BASE_KEY_STRATEGY_TYPE,
                                   BRStrategyType::BR_STRATEGY_TYPE_SYSTEM);

    if (retval >= BRStrategyType::BR_STRATEGY_TYPE_LAST || retval < 0)
    {
        KLOG_WARNING("The strategy type is invalid. strategy type: %d.", retval);
        return BRStrategyType::BR_STRATEGY_TYPE_SYSTEM;
    }

    return BRStrategyType(retval);
}

bool Configuration::setStrategyType(BRStrategyType strategyType)
{
    RETURN_VAL_IF_FALSE(strategyType < BRStrategyType::BR_STRATEGY_TYPE_LAST, false);
    RETURN_VAL_IF_TRUE(strategyType == this->getStrategyType(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_STRATEGY_TYPE, int32_t(strategyType));

    Q_EMIT StrategyChanged();
    return true;
}

QSharedPointer<Protocol::RA> Configuration::getCustomRA()
{
    if (this->getStrategyType() == BRStrategyType::BR_STRATEGY_TYPE_SYSTEM)
    {
        return QSharedPointer<Protocol::RA>();
    }

    return this->readRaFromFile();
}

bool Configuration::setCustomRA(const Protocol::Reinforcement& reinforcement)
{
    if (this->getStrategyType() == BRStrategyType::BR_STRATEGY_TYPE_SYSTEM)
    {
        KLOG_WARNING() << "Current is system strategy, so not allow to set custom reinforcement arguments";
        return false;
    }

    auto ra = this->readRaFromFile();

    bool matchReinforcement = false;

    auto& reinforcements = ra->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        CONTINUE_IF_TRUE(iter->name() != reinforcement.name());
        matchReinforcement = true;
        auto& newArgs = reinforcement.arg();
        for (auto newArgIter = newArgs.begin(); newArgIter != newArgs.end(); ++newArgIter)
        {
            auto& oldArgs = iter->arg();
            for (auto oldArgIter = oldArgs.begin(); oldArgIter != oldArgs.end(); ++oldArgIter)
            {
                CONTINUE_IF_TRUE(oldArgIter->name() != newArgIter->name());
                oldArgIter->value(newArgIter->value());
                break;
            }
        }
        break;
    }

    // 如果配置中不存在加固项的自定义配置，则添加该加固项的自定义配置
    if (!matchReinforcement)
    {
        Protocol::Reinforcement used_reinforcement(reinforcement.name());

        const auto& args = reinforcement.arg();

        for (auto iter = args.begin(); iter != args.end(); ++iter)
        {
            auto& arg = (*iter);
            Protocol::ReinforcementArg used_arg(arg.name(), arg.value());
            used_reinforcement.arg().push_back(used_arg);
        }
        ra->reinforcement().push_back(used_reinforcement);
    }

    return this->writeRAToFile(ra);
}

bool Configuration::delCustomRA(const QString& name)
{
    if (this->getStrategyType() == BRStrategyType::BR_STRATEGY_TYPE_SYSTEM)
    {
        KLOG_WARNING() << "Current is system strategy, so not allow to delete custom reinforcement arguments";
        return false;
    }

    auto ra = this->readRaFromFile();
    bool isDeleted = false;

    for (auto iter = ra->reinforcement().begin(); iter != ra->reinforcement().end(); ++iter)
    {
        if (iter->name() == name.toStdString())
        {
            ra->reinforcement().erase(iter);
            isDeleted = true;
            break;
        }
    }

    if (isDeleted)
    {
        return this->writeRAToFile(ra);
    }

    return false;
}

void Configuration::delAllCustomRA()
{
    if (this->getStrategyType() == BRStrategyType::BR_STRATEGY_TYPE_SYSTEM)
    {
        KLOG_WARNING() << "Current is system strategy, so not allow to delete all custom reinforcement arguments";
        return;
    }

    auto ra = this->readRaFromFile();

    if (ra->reinforcement().size() > 0)
    {
        ra->reinforcement().clear();
        this->writeRAToFile(ra);
    }
}

int Configuration::getTimeScan()
{
    auto retval = this->getInteger(BR_GROUP_NAME, BR_BASE_KEY_TIME_SCAN);

    if (retval < 0)
    {
        KLOG_WARNING("The strategy type is invalid. time scan: %d.", retval);
        return 0;
    }

    return int(retval);
}

bool Configuration::setTimeScan(int time_scan)
{
    // RETURN_VAL_IF_FALSE(time_scan < 99, false);
    RETURN_VAL_IF_TRUE(time_scan == this->getTimeScan(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_TIME_SCAN, int32_t(time_scan));
    return true;
}

BRNotificationStatus Configuration::getNotificationStatus()
{
    auto retval = this->getInteger(BR_GROUP_NAME,
                                   BR_BASE_KEY_NOTIFICATION_STATUS,
                                   BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN);

    if (retval >= BRNotificationStatus::BR_NOTIFICATION_STATUS_OTHER || retval < 0)
    {
        KLOG_WARNING("The strategy type is invalid. notification status: %d.", retval);
        return BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN;
    }

    return BRNotificationStatus(retval);
}

bool Configuration::setNotificationStatus(BRNotificationStatus notificationStatus)
{
    RETURN_VAL_IF_FALSE(notificationStatus < BRNotificationStatus::BR_NOTIFICATION_STATUS_OTHER, false);
    RETURN_VAL_IF_TRUE(notificationStatus == this->getNotificationStatus(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_NOTIFICATION_STATUS, int32_t(notificationStatus));
    return true;
}

BRFallbackStatus Configuration::getFallbackStatus()
{
    auto retval = this->getInteger(BR_GROUP_NAME,
                                   BR_BASE_KEY_FALLBACK_STATUS,
                                   BRFallbackStatus::BR_FALLBACK_STATUS_NOT_STARTED);

    if (retval > BRFallbackStatus::BR_FALLBACK_STATUS_IS_FINISHED || retval < 0)
    {
        KLOG_WARNING("The strategy type is invalid. notification status: %d.", retval);
        return BRFallbackStatus::BR_FALLBACK_STATUS_NOT_STARTED;
    }

    return BRFallbackStatus(retval);
}
bool Configuration::setFallbackStatus(BRFallbackStatus fallbackStatus)
{
    RETURN_VAL_IF_TRUE(fallbackStatus > BRFallbackStatus::BR_FALLBACK_STATUS_IS_FINISHED, false);
    RETURN_VAL_IF_TRUE(fallbackStatus == this->getFallbackStatus(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_FALLBACK_STATUS, int32_t(fallbackStatus));
    return true;
}

bool Configuration::checkRaStrategy()
{
    try
    {
        std::make_shared<Protocol::RA>(*br_ra(CUSTOM_RA_STRATEGY_FILEPATH, xml_schema::Flags::dont_validate));
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        return false;
    }

    return true;
}

void Configuration::setRaCheckbox(const QString& name, const bool& status)
{
    auto ra = this->readRaFromFile();
    auto& reinforcements = ra->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        if (name.toStdString() == iter->name())
            iter->checkbox().set(status);
    }
    writeRAToFile(ra);
}

QSharedPointer<Protocol::RS> Configuration::getRS()
{
    return this->getFixedRS();
}

bool Configuration::setCustomRS(const QString& encryptedRS, SSRErrorCode& errorCode)
{
    // 判断自定义加固标准
    auto decryptedRS = CryptoHelper::brDecrypt(RSA_PUBLIC_KEY_FILEPATH, encryptedRS);
    if (decryptedRS.isEmpty())
    {
        errorCode = SSRErrorCode::ERROR_CUSTOM_RS_DECRYPT_FAILED;
        return false;
    }

    QFile file(CUSTOM_RS_FILEPATH);
    // 文件打开成功才会继续写内容,如果都成功则是不进入 if , 继续执行
    if (!(file.open(QIODevice::OpenModeFlag::ReadWrite | QIODevice::OpenModeFlag::Truncate) != false &&
          file.write(encryptedRS.toLatin1()) != -1))
    {
        return false;
    }

    Q_EMIT RSChanged();
    return true;
}

BRResourceMonitor Configuration::getResourceMonitorStatus()
{
    auto retval = this->getInteger(BR_GROUP_NAME,
                                   BR_BASE_KEY_RESOURCE_MONITOR,
                                   BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN);

    if (retval >= BRResourceMonitor::BR_RESOURCE_MONITOR_OTHER || retval < 0)
    {
        KLOG_WARNING("The resource monitor is invalid. resource monitor: %d.", retval);
        return BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN;
    }

    return BRResourceMonitor(retval);
}

bool Configuration::setResourceMonitorStatus(BRResourceMonitor resourceMonitor)
{
    RETURN_VAL_IF_FALSE(resourceMonitor < BRResourceMonitor::BR_RESOURCE_MONITOR_OTHER, false);
    RETURN_VAL_IF_TRUE(resourceMonitor == this->getResourceMonitorStatus(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_RESOURCE_MONITOR, int32_t(resourceMonitor));
    return true;
}

void Configuration::init()
{
    KLOG_DEBUG("Configuration::init");
    this->m_settings = new QSettings(this->m_configPath, QSettings::NativeFormat);
}

QSharedPointer<Protocol::RS> Configuration::getFixedRS()
{
    KLOG_DEBUG("Configuration::getFixedRs");

    QString rsFilePath = (this->getStandardType() == BRStandardType::BR_STANDARD_TYPE_CUSTOM) ? CUSTOM_RS_FILEPATH : SYSTEM_RS_FILEPATH;

    // 加载加固标准
    try
    {
        auto rsDecrypted = this->decryptFile(rsFilePath);
        KLOG_DEBUG() << "rs file decrypted: " << rsDecrypted.toLocal8Bit();
        std::istringstream rs_istream(rsDecrypted.toStdString());
        return QSharedPointer<Protocol::RS>(new Protocol::RS(*br_rs(rs_istream, xml_schema::Flags::dont_validate)));
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
    }
    return QSharedPointer<Protocol::RS>();
}

QSharedPointer<Protocol::RA> Configuration::readRaFromFile()
{
    KLOG_DEBUG("Configuration::readRaFromFile");

    RETURN_VAL_IF_TRUE(!QFileInfo(CUSTOM_RA_FILEPATH).isFile(),
                       QSharedPointer<RA>(new RA()));

    try
    {
        return QSharedPointer<Protocol::RA>(new Protocol::RA(*br_ra(CUSTOM_RA_FILEPATH, xml_schema::Flags::dont_validate)));
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
    }
    return QSharedPointer<Protocol::RA>(new Protocol::RA());
}

bool Configuration::writeRAToFile(QSharedPointer<Protocol::RA> ra)
{
    try
    {
        std::ofstream ofs(CUSTOM_RA_FILEPATH, std::ios_base::out);
        br_ra(ofs, *ra.data());
        ofs.close();
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        return false;
    }

    Q_EMIT customRAChanged();
    return true;
}

std::shared_ptr<Protocol::ReinforcementHistory> Configuration::readRhFromFile(const QString path)
{
    RETURN_VAL_IF_TRUE(!QFileInfo(path).isFile(),
                       std::make_shared<ReinforcementHistory>());

    try
    {
        return std::make_shared<Protocol::ReinforcementHistory>(*br_rh(path.toStdString(),
                                                                       xml_schema::Flags::dont_validate));
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
    }
    return std::make_shared<Protocol::ReinforcementHistory>();
}

bool Configuration::writeRhToFile(std::shared_ptr<Protocol::ReinforcementHistory> rh, const QString path)
{
    try
    {
        std::ofstream ofs(path.toStdString(), std::ios_base::out);
        br_rh(ofs, *rh.get());
        ofs.close();
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        return false;
    }
    return true;
}

bool Configuration::setCustomRh(const Reinforcement& rsReinforcement, const QString path)
{
    auto rh = this->readRhFromFile(path);

    bool matchReinforcement = false;

    auto& reinforcements = rh->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        CONTINUE_IF_TRUE(iter->name() != rsReinforcement.name());

        matchReinforcement = true;
        auto& new_args = rsReinforcement.arg();
        for (auto new_arg_iter = new_args.begin(); new_arg_iter != new_args.end(); ++new_arg_iter)
        {
            auto& old_args = iter->arg();
            for (auto old_arg_iter = old_args.begin(); old_arg_iter != old_args.end(); ++old_arg_iter)
            {
                CONTINUE_IF_TRUE(old_arg_iter->name() != new_arg_iter->name());
                old_arg_iter->value(new_arg_iter->value());
                break;
            }
        }
        break;
    }

    // 如果配置中不存在加固项的自定义配置，则添加该加固项的自定义配置
    if (!matchReinforcement)
    {
        Protocol::Reinforcement used_reinforcement(rsReinforcement.name());

        const auto& args = rsReinforcement.arg();

        for (auto iter = args.begin(); iter != args.end(); ++iter)
        {
            auto& arg = (*iter);
            Protocol::ReinforcementArg used_arg(arg.name(), arg.value());
            used_reinforcement.arg().push_back(used_arg);
        }
        rh->reinforcement().push_back(used_reinforcement);
    }

    return this->writeRhToFile(rh, path);
}

void Configuration::joinReinforcement(Reinforcement& to_r, const Reinforcement& from_r)
{
    KLOG_DEBUG("Join reinforcement %s.", from_r.name().c_str());

    const auto& from_args = from_r.arg();
    for (auto from_arg_iter = from_args.begin(); from_arg_iter != from_args.end(); ++from_arg_iter)
    {
        auto& to_args = to_r.arg();
        for (auto to_arg_iter = to_args.begin(); to_arg_iter != to_args.end(); ++to_arg_iter)
        {
            CONTINUE_IF_TRUE(from_arg_iter->name() != to_arg_iter->name());
            KLOG_DEBUG("New argument: %s, old argument: %s.", from_arg_iter->value().c_str(), to_arg_iter->value().c_str());
            to_arg_iter->value(from_arg_iter->value());
            break;
        }
    }
}

QString Configuration::decryptFile(const QString& filename)
{
    KLOG_DEBUG() << "filename: " << filename.toLocal8Bit();

    RETURN_VAL_IF_TRUE(filename.isEmpty(), QString());
    RETURN_VAL_IF_TRUE(!QFileInfo(filename).isFile(), QString());

    QFile file(filename);
    if (!file.open(QIODevice::OpenModeFlag::ReadOnly))
    {
        KLOG_WARNING() << "failed to open file: " << filename << " fa.";
    }
    auto encrypted_contents = file.readAll();
    return CryptoHelper::brDecrypt(RSA_PUBLIC_KEY_FILEPATH, encrypted_contents);
}

int32_t Configuration::getInteger(const QString& groupName, const QString& key, int32_t defaultValue)
{
    return this->m_settings->value(groupName + '/' + key, defaultValue).toInt();
}

QString Configuration::getString(const QString& groupName, const QString& key)
{
    return this->m_settings->value(groupName + '/' + key).toString();
}

QString Configuration::getDatadirFilename(const QString& groupName, const QString& key)
{
    auto basename = this->getString(groupName, key);
    RETURN_VAL_IF_TRUE(basename.isEmpty(), QString());
    return QDir::cleanPath(SSR_INSTALL_DATADIR + basename);
}

void Configuration::setInteger(const QString& groupName, const QString& key, int32_t value)
{
    this->m_settings->setValue(groupName + '/' + key, value);
}

void Configuration::setString(const QString& groupName, const QString& key, const QString& value)
{
    this->m_settings->setValue(groupName + '/' + key, value);
}

}  // namespace BR
}  // namespace KS
