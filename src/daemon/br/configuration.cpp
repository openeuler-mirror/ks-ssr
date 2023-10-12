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

#include "configuration.h"
#include <fstream>

namespace KS
{
namespace BRDaemon
{
#define BR_GROUP_NAME "br"
#define BR_BASE_KEY_MAX_THREAD_NUM "max_thread_num"
#define BR_BASE_KEY_STANDARD_TYPE "standard_type"
#define BR_BASE_KEY_STRATEGY_TYPE "strategy_type"
#define BR_BASE_KEY_RESOURCE_MONITOR "resource_monitor"
#define BR_BASE_KEY_TIME_SCAN "time_scan"
#define BR_BASE_KEY_NOTIFICATION_STATUS "notification_status"

#define MAX_THREAD_NUM_DEFAULT 1

#define SYSTEM_RS_FILEPATH SSR_BR_INSTALL_DATADIR "/br-system-rs"
#define CUSTOM_RS_FILEPATH SSR_BR_INSTALL_DATADIR "/br-custom-rs"

#define CUSTOM_RA_FILEPATH SSR_BR_INSTALL_DATADIR "/br-custom-ra.xml"
#define CUSTOM_RA_STRATEGY_FILEPATH SSR_BR_INSTALL_DATADIR "/br-custom-ra-strategy.xml"
#define RH_BR_DATDIR SSR_BR_INSTALL_DATADIR "/ReinforcementHistory"
#define RSA_PUBLIC_KEY_FILEPATH SSR_BR_INSTALL_DATADIR "/br-public.key"

using namespace Protocol;

Configuration::Configuration(const QString& config_path) : config_path_(config_path)
{
}

Configuration::~Configuration()
{
    delete this->configuration_;
}

Configuration* Configuration::instance_ = NULL;
void Configuration::globalInit(const QString& config_path)
{
    instance_ = new Configuration(config_path);
    instance_->init();
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

bool Configuration::setStandardType(BRStandardType standard_type)
{
    RETURN_VAL_IF_FALSE(standard_type < BRStandardType::BR_STANDARD_TYPE_LAST, false);
    RETURN_VAL_IF_TRUE(standard_type == this->getStandardType(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_STANDARD_TYPE, int32_t(standard_type));
    // if (!this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_STANDARD_TYPE, int32_t(standard_type)))
    // {
    //     KLOG_WARNING("Failed to set standard type.");
    //     return false;
    // }
    this->reloadRs();
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

bool Configuration::setStrategyType(BRStrategyType strategy_type)
{
    RETURN_VAL_IF_FALSE(strategy_type < BRStrategyType::BR_STRATEGY_TYPE_LAST, false);
    RETURN_VAL_IF_TRUE(strategy_type == this->getStrategyType(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_STRATEGY_TYPE, int32_t(strategy_type));
    // if (!this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_STRATEGY_TYPE, int32_t(strategy_type)))
    // {
    //     KLOG_WARNING("Failed to set strategy type.");
    //     return false;
    // }
    // this->reloadStrategy();
    return true;
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
    RETURN_VAL_IF_TRUE(time_scan == this->getNotificationStatus(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_TIME_SCAN, int32_t(time_scan));
    return true;
    // if (!this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_TIME_SCAN, int32_t(time_scan)))
    // {
    //     KLOG_WARNING("Failed to set time scan.");
    //     return false;
    // }
    // // this->reloadStrategy();
    // return true;
}

BRNotificationStatus Configuration::getNotificationStatus()
{
    auto retval = this->getInteger(BR_GROUP_NAME,
                                   BR_BASE_KEY_NOTIFICATION_STATUS,
                                   BRNotificationStatus::BR_NOTIFICATION_OPEN);

    if (retval >= BRNotificationStatus::BR_NOTIFICATION_OTHER || retval < 0)
    {
        KLOG_WARNING("The strategy type is invalid. notification status: %d.", retval);
        return BRNotificationStatus::BR_NOTIFICATION_OPEN;
    }

    return BRNotificationStatus(retval);
}

bool Configuration::setNotificationStatus(BRNotificationStatus notification_status)
{
    RETURN_VAL_IF_FALSE(notification_status < BRNotificationStatus::BR_NOTIFICATION_OTHER, false);
    RETURN_VAL_IF_TRUE(notification_status == this->getNotificationStatus(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_NOTIFICATION_STATUS, int32_t(notification_status));
    return true;
    // if (!this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_NOTIFICATION_STATUS, int32_t(notification_status)))
    // {
    //     KLOG_WARNING("Failed to set notification status.");
    //     return false;
    // }
    // // this->reloadStrategy();
    // return true;
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
    writeRaToFile(ra);
}

bool Configuration::setCustomRs(const QString& encrypted_rs, BRErrorCode& error_code)
{
    // 判断自定义加固标准
    auto decrypted_rs = CryptoHelper::brDecrypt(RSA_PUBLIC_KEY_FILEPATH, encrypted_rs);
    if (decrypted_rs.isEmpty())
    {
        error_code = BRErrorCode::ERROR_CUSTOM_RS_DECRYPT_FAILED;
        return false;
    }

    QFile file(CUSTOM_RS_FILEPATH);
    // 文件打开成功才会继续写内容,如果都成功则是不进入 if , 继续执行
    if (!(file.open(QIODevice::OpenModeFlag::ReadWrite | QIODevice::OpenModeFlag::Truncate) != false &&
          file.write(encrypted_rs.toLatin1()) != -1))
    {
        return false;
    }
    if (this->getStandardType() == BRStandardType::BR_STANDARD_TYPE_CUSTOM)
    {
        this->reloadRs();
    }
    return true;
}

bool Configuration::setCustomRa(const Protocol::Reinforcement& rs_reinforcement)
{
    auto ra = this->readRaFromFile();

    bool match_reinforcement = false;

    auto& reinforcements = ra->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        CONTINUE_IF_TRUE(iter->name() != rs_reinforcement.name());
        // iter->checkbox().set("unchecked");
        match_reinforcement = true;
        auto& new_args = rs_reinforcement.arg();
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
    if (!match_reinforcement)
    {
        Protocol::Reinforcement used_reinforcement(rs_reinforcement.name());

        const auto& args = rs_reinforcement.arg();

        for (auto iter = args.begin(); iter != args.end(); ++iter)
        {
            auto& arg = (*iter);
            Protocol::ReinforcementArg used_arg(arg.name(), arg.value());
            used_reinforcement.arg().push_back(used_arg);
            // used_reinforcement.checkbox().set("unchecked");
        }
        ra->reinforcement().push_back(used_reinforcement);
    }

    return this->writeRaToFile(ra);
}

void Configuration::delCustomRa(const QString& name)
{
    KLOG_DEBUG("delCustomRa name = %s", name.toLatin1());
    auto ra = this->readRaFromFile();
    bool is_del = false;

    for (auto iter = ra->reinforcement().begin(); iter != ra->reinforcement().end(); ++iter)
    {
        if (iter->name() == name.toStdString())
        {
            ra->reinforcement().erase(iter);
            is_del = true;
            break;
        }
    }

    if (is_del)
    {
        this->writeRaToFile(ra);
    }
}

void Configuration::delAllCustomRa()
{
    auto ra = this->readRaFromFile();

    if (ra->reinforcement().size() > 0)
    {
        ra->reinforcement().clear();
        this->writeRaToFile(ra);
    }
}

BRResourceMonitor Configuration::getResourceMonitorStatus()
{
    auto retval = this->getInteger(BR_GROUP_NAME,
                                   BR_BASE_KEY_RESOURCE_MONITOR,
                                   BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN);

    if (retval >= BRResourceMonitor::BR_RESOURCE_MONITOR_OR || retval < 0)
    {
        KLOG_WARNING("The resource monitor is invalid. resource monitor: %d.", retval);
        return BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN;
    }

    return BRResourceMonitor(retval);
}

bool Configuration::setResourceMonitorStatus(BRResourceMonitor resource_monitor)
{
    RETURN_VAL_IF_FALSE(resource_monitor < BRResourceMonitor::BR_RESOURCE_MONITOR_OR, false);
    RETURN_VAL_IF_TRUE(resource_monitor == this->getResourceMonitorStatus(), true);

    this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_RESOURCE_MONITOR, int32_t(resource_monitor));
    return true;
    // if (!this->setInteger(BR_GROUP_NAME, BR_BASE_KEY_RESOURCE_MONITOR, int32_t(resource_monitor)))
    // {
    //     KLOG_WARNING("Failed to set resource monitor.");
    //     return false;
    // }
    // return true;
}

void Configuration::init()
{
    KLOG_DEBUG("");
    this->configuration_ = new QSettings(this->config_path_, QSettings::NativeFormat);
    this->loadRs();
}

void Configuration::reloadRs()
{
    this->loadRs();
    emit this->rs_changed_();
}

void Configuration::loadRs()
{
    this->rs_ = this->getFixedRs();
    RETURN_IF_FALSE(this->rs_);

    auto ra = this->readRaFromFile();
    // 将固定不变的加固标准部分和用户修改的自定义部分进行整合
    auto& custom_reinforcements = ra->reinforcement();
    for (auto custom_iter = custom_reinforcements.begin(); custom_iter != custom_reinforcements.end(); ++custom_iter)
    {
        auto& fixed_reinforcements = this->rs_->body().reinforcement();
        for (auto fixed_iter = fixed_reinforcements.begin(); fixed_iter != fixed_reinforcements.end(); ++fixed_iter)
        {
            CONTINUE_IF_TRUE(custom_iter->name() != fixed_iter->name());
            this->joinReinforcement((*fixed_iter), (*custom_iter));
        }
    }
}

// void Configuration::reloadStrategy()
// {
//     this->rs_ = this->getFixedRs();
//     RETURN_IF_FALSE(this->rs_);

//     auto ra_strategy = this->readRaFromFile(CUSTOM_RA_STRATEGY_FILEPATH);
//     // 将固定不变的加固标准部分和用户修改的自定义部分进行整合
//     auto& custom_reinforcements = ra_strategy->reinforcement();
//     for (auto custom_iter = custom_reinforcements.begin(); custom_iter != custom_reinforcements.end(); ++custom_iter)
//     {
//         auto& fixed_reinforcements = this->rs_->body().reinforcement();
//         for (auto fixed_iter = fixed_reinforcements.begin(); fixed_iter != fixed_reinforcements.end(); ++fixed_iter)
//         {
//             CONTINUE_IF_TRUE(custom_iter->name() != fixed_iter->name());
//             this->joinReinforcement((*fixed_iter), (*custom_iter));
//         }
//     }
//     this->rs_changed_.emit();
// }

QSharedPointer<Protocol::RS> Configuration::getFixedRs()
{
    KLOG_DEBUG("");

    QString rs_file_path = (this->getStandardType() == BRStandardType::BR_STANDARD_TYPE_CUSTOM) ? CUSTOM_RS_FILEPATH : SYSTEM_RS_FILEPATH;

    // 加载加固标准
    try
    {
        auto rs_decrypted = this->decryptFile(rs_file_path);
        KLOG_DEBUG() << "rs file decrypted: " << rs_decrypted.toLocal8Bit();
        std::istringstream rs_istream(rs_decrypted.toStdString());
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
    KLOG_DEBUG("");

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

bool Configuration::writeRaToFile(QSharedPointer<Protocol::RA> ra)
{
    try
    {
        std::ofstream ofs(CUSTOM_RA_FILEPATH, std::ios_base::out);
        br_ra(ofs, *ra.get());
        ofs.close();
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        return false;
    }
    this->reloadRs();
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
    //    this->reloadRs();
    return true;
}

bool Configuration::setCustomRh(const Reinforcement& rs_reinforcement, const QString path)
{
    auto rh = this->readRhFromFile(path);

    bool match_reinforcement = false;

    auto& reinforcements = rh->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        CONTINUE_IF_TRUE(iter->name() != rs_reinforcement.name());

        match_reinforcement = true;
        auto& new_args = rs_reinforcement.arg();
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
    if (!match_reinforcement)
    {
        Protocol::Reinforcement used_reinforcement(rs_reinforcement.name());

        const auto& args = rs_reinforcement.arg();

        for (auto iter = args.begin(); iter != args.end(); ++iter)
        {
            auto& arg = (*iter);
            Protocol::ReinforcementArg used_arg(arg.name(), arg.value());
            used_reinforcement.arg().push_back(used_arg);
            // used_reinforcement.checkbox().set("unchecked");
        }
        //        ra->reinforcement().
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
    KLOG_DEBUG("filename: %s.", filename.toLocal8Bit());

    RETURN_VAL_IF_TRUE(filename.isEmpty(), QString());
    RETURN_VAL_IF_TRUE(!QFileInfo(filename).isFile(), QString());

    QFile file(filename);
    if (!file.open(QIODevice::OpenModeFlag::ReadOnly))
    {
        KLOG_WARNING("failed to open file: %s fa.", filename);
    }
    auto encrypted_contents = file.readAll();
    return CryptoHelper::brDecrypt(RSA_PUBLIC_KEY_FILEPATH, encrypted_contents);
}

int32_t Configuration::getInteger(const QString& group_name, const QString& key, int32_t default_value)
{
    int32_t retval = default_value;
    // IGNORE_EXCEPTION(retval = this->configuration_.getInteger(group_name, key));
    retval = this->configuration_->value(group_name + '/' + key).toInt();
    return retval;
}

QString Configuration::getString(const QString& group_name, const QString& key)
{
    QString retval;
    // IGNORE_EXCEPTION(retval = this->configuration_.getString(group_name, key));
    retval = this->configuration_->value(group_name + '/' + key).toInt();
    return retval;
}

QString Configuration::getDatadirFilename(const QString& group_name, const QString& key)
{
    auto basename = this->getString(group_name, key);
    RETURN_VAL_IF_TRUE(basename.isEmpty(), QString());
    // return Glib::build_filename(SSR_BR_INSTALL_DATADIR, basename);
    return QDir::cleanPath(SSR_BR_INSTALL_DATADIR + basename);
}

void Configuration::setInteger(const QString& group_name, const QString& key, int32_t value)
{
    // this->configuration_.set_integer(group_name, key, value);
    this->configuration_->setValue(group_name + '/' + key, value);
    // return this->saveToFile();
}

void Configuration::setString(const QString& group_name, const QString& key, const QString& value)
{
    // this->configuration_.setString(group_name, key, value);
    this->configuration_->setValue(group_name + '/' + key, value);
    // return this->saveToFile();
}

}  // namespace BRDaemon
}  // namespace KS
