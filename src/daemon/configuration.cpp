/**
 * @file          /ks-ssr-manager/src/daemon/configuration.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/configuration.h"
#include <fstream>

namespace KS
{
namespace Daemon
{
#define SSR_GROUP_NAME "base"
#define SSR_BASE_KEY_MAX_THREAD_NUM "max_thread_num"
#define SSR_BASE_KEY_STANDARD_TYPE "standard_type"

#define MAX_THREAD_NUM_DEFAULT 1

#define SYSTEM_RS_FILEPATH SSR_INSTALL_DATADIR "/ssr-system-rs"
#define CUSTOM_RS_FILEPATH SSR_INSTALL_DATADIR "/ssr-custom-rs"

#define CUSTOM_RA_FILEPATH SSR_INSTALL_DATADIR "/ssr-custom-ra.xml"
#define RSA_PUBLIC_KEY_FILEPATH SSR_INSTALL_DATADIR "/ssr-public.key"

using namespace Protocol;

Configuration::Configuration(const std::string& config_path) : config_path_(config_path)
{
}

Configuration::~Configuration()
{
}

Configuration* Configuration::instance_ = NULL;
void Configuration::global_init(const std::string& config_path)
{
    instance_ = new Configuration(config_path);
    instance_->init();
}

uint32_t Configuration::get_max_thread_num()
{
    return this->get_integer(SSR_GROUP_NAME, SSR_BASE_KEY_MAX_THREAD_NUM, MAX_THREAD_NUM_DEFAULT);
}

SSRStandardType Configuration::get_standard_type()
{
    auto retval = this->get_integer(SSR_GROUP_NAME,
                                    SSR_BASE_KEY_STANDARD_TYPE,
                                    SSRStandardType::SSR_STANDARD_TYPE_SYSTEM);

    if (retval >= SSR_STANDARD_TYPE_LAST || retval < 0)
    {
        KLOG_WARNING("The standard type is invalid. standard type: %d.", retval);
        return SSRStandardType::SSR_STANDARD_TYPE_SYSTEM;
    }

    return SSRStandardType(retval);
}

bool Configuration::set_standard_type(SSRStandardType standard_type)
{
    RETURN_VAL_IF_FALSE(standard_type < SSRStandardType::SSR_STANDARD_TYPE_LAST, false);
    RETURN_VAL_IF_TRUE(standard_type == this->get_standard_type(), true);

    if (!this->set_integer(SSR_GROUP_NAME, SSR_BASE_KEY_STANDARD_TYPE, int32_t(standard_type)))
    {
        KLOG_WARNING("Failed to set standard type.");
        return false;
    }
    this->reload_rs();
    return true;
}

bool Configuration::set_custom_rs(const std::string& encrypted_rs, SSRErrorCode& error_code)
{
    // 判断自定义加固标准
    auto decrypted_rs = CryptoHelper::ssr_decrypt(RSA_PUBLIC_KEY_FILEPATH, encrypted_rs);
    if (decrypted_rs.empty())
    {
        error_code = SSRErrorCode::ERROR_CUSTOM_RS_DECRYPT_FAILED;
        return false;
    }

    try
    {
        Glib::file_set_contents(CUSTOM_RS_FILEPATH, encrypted_rs);
        if (this->get_standard_type() == SSRStandardType::SSR_STANDARD_TYPE_CUSTOM)
        {
            this->reload_rs();
        }
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
        return false;
    }
    return true;
}

bool Configuration::set_custom_ra(const Protocol::Reinforcement& rs_reinforcement)
{
    auto ra = this->read_ra_from_file();

    bool match_reinforcement = false;

    auto& reinforcements = ra->reinforcement();
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
        }

        ra->reinforcement().push_back(used_reinforcement);
    }

    return this->write_ra_to_file(ra);
}

void Configuration::del_custom_ra(const std::string& name)
{
    auto ra = this->read_ra_from_file();
    bool is_del = false;

    for (auto iter = ra->reinforcement().begin(); iter != ra->reinforcement().end(); ++iter)
    {
        if (iter->name() == name)
        {
            ra->reinforcement().erase(iter);
            is_del = true;
            break;
        }
    }

    if (is_del)
    {
        this->write_ra_to_file(ra);
    }
}

void Configuration::del_all_custom_ra()
{
    auto ra = this->read_ra_from_file();

    if (ra->reinforcement().size() > 0)
    {
        ra->reinforcement().clear();
        this->write_ra_to_file(ra);
    }
}

void Configuration::init()
{
    KLOG_PROFILE("");

    try
    {
        this->configuration_.load_from_file(this->config_path_);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
        return;
    }

    this->load_rs();
}

void Configuration::reload_rs()
{
    this->load_rs();
    this->rs_changed_.emit();
}

void Configuration::load_rs()
{
    this->rs_ = this->get_fixed_rs();
    RETURN_IF_FALSE(this->rs_);

    auto ra = this->read_ra_from_file();
    // 将固定不变的加固标准部分和用户修改的自定义部分进行整合
    auto& custom_reinforcements = ra->reinforcement();
    for (auto custom_iter = custom_reinforcements.begin(); custom_iter != custom_reinforcements.end(); ++custom_iter)
    {
        auto& fixed_reinforcements = this->rs_->body().reinforcement();
        for (auto fixed_iter = fixed_reinforcements.begin(); fixed_iter != fixed_reinforcements.end(); ++fixed_iter)
        {
            CONTINUE_IF_TRUE(custom_iter->name() != fixed_iter->name());
            this->join_reinforcement((*fixed_iter), (*custom_iter));
        }
    }
}

std::shared_ptr<Protocol::RS> Configuration::get_fixed_rs()
{
    KLOG_PROFILE("");

    std::string rs_file_path = (this->get_standard_type() == SSRStandardType::SSR_STANDARD_TYPE_CUSTOM) ? CUSTOM_RS_FILEPATH : SYSTEM_RS_FILEPATH;

    // 加载加固标准
    try
    {
        auto rs_decrypted = this->decrypt_file(rs_file_path);
        KLOG_DEBUG("%s", rs_decrypted.c_str());
        std::istringstream rs_istream(rs_decrypted);
        return std::make_shared<Protocol::RS>(*ssr_rs(rs_istream, xml_schema::Flags::dont_validate));
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
    }
    return std::shared_ptr<Protocol::RS>();
}

std::shared_ptr<Protocol::RA> Configuration::read_ra_from_file()
{
    KLOG_PROFILE("");

    RETURN_VAL_IF_TRUE(!Glib::file_test(CUSTOM_RA_FILEPATH, Glib::FILE_TEST_IS_REGULAR),
                       std::make_shared<RA>());

    try
    {
        return std::make_shared<Protocol::RA>(*ssr_ra(CUSTOM_RA_FILEPATH, xml_schema::Flags::dont_validate));
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
    }
    return std::make_shared<Protocol::RA>();
}

bool Configuration::write_ra_to_file(std::shared_ptr<Protocol::RA> ra)
{
    try
    {
        std::ofstream ofs(CUSTOM_RA_FILEPATH, std::ios_base::out);
        ssr_ra(ofs, *ra.get());
        ofs.close();
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        return false;
    }
    this->reload_rs();
    return true;
}

void Configuration::join_reinforcement(Reinforcement& to_r, const Reinforcement& from_r)
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

std::string Configuration::decrypt_file(const std::string& filename)
{
    KLOG_PROFILE("filename: %s.", filename.c_str());

    RETURN_VAL_IF_TRUE(filename.empty(), std::string());
    RETURN_VAL_IF_TRUE(!Glib::file_test(filename, Glib::FILE_TEST_IS_REGULAR), std::string());

    try
    {
        auto encrypted_contents = Glib::file_get_contents(filename);
        return CryptoHelper::ssr_decrypt(RSA_PUBLIC_KEY_FILEPATH, encrypted_contents);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }
    return std::string();
}

int32_t Configuration::get_integer(const std::string& group_name, const std::string& key, int32_t default_value)
{
    int32_t retval = default_value;
    IGNORE_EXCEPTION(retval = this->configuration_.get_integer(group_name, key));
    return retval;
}

std::string Configuration::get_string(const std::string& group_name, const std::string& key)
{
    std::string retval;
    IGNORE_EXCEPTION(retval = this->configuration_.get_string(group_name, key));
    return retval;
}

std::string Configuration::get_datadir_filename(const std::string& group_name, const std::string& key)
{
    auto basename = this->get_string(group_name, key);
    RETURN_VAL_IF_TRUE(basename.empty(), std::string());
    return Glib::build_filename(SSR_INSTALL_DATADIR, basename);
}

bool Configuration::set_integer(const std::string& group_name, const std::string& key, int32_t value)
{
    this->configuration_.set_integer(SSR_GROUP_NAME, SSR_BASE_KEY_STANDARD_TYPE, value);
    return this->save_to_file();
}

bool Configuration::set_string(const std::string& group_name, const std::string& key, const std::string& value)
{
    this->configuration_.set_string(group_name, key, value);
    return this->save_to_file();
}

bool Configuration::save_to_file()
{
    try
    {
        auto data = this->configuration_.to_data();
        Glib::file_set_contents(this->config_path_, data);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
        return false;
    }
    return true;
}

}  // namespace Daemon
}  // namespace KS
