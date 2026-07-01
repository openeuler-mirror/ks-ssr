/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-configuration.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/ssr-configuration.h"
#include "ssr-config.h"

namespace Kiran
{
#define SSR_GROUP_NAME "base"
#define SSR_BASE_KEY_MAX_THREAD_NUM "max_thread_num"
#define SSR_BASE_KEY_STANDARD_TYPE "standard_type"

#define MAX_THREAD_NUM_DEFAULT 1
#define SYSTEM_RS_FILEPATH SSR_INSTALL_DATADIR "/ssr-system-rs.json"
#define CUSTOM_RS_FILEPATH SSR_INSTALL_DATADIR "/ssr-custom-rs.json"
#define CUSTOM_RA_FILEPATH SSR_INSTALL_DATADIR "/ssr-custom-ra.json"
#define RSA_PUBLIC_KEY_FILEPATH SSR_INSTALL_DATADIR "/ssr-public.key"

SSRConfiguration::SSRConfiguration(const std::string& config_path) : config_path_(config_path)
{
}

SSRConfiguration::~SSRConfiguration()
{
}

SSRConfiguration* SSRConfiguration::instance_ = nullptr;
void SSRConfiguration::global_init(const std::string& config_path)
{
    instance_ = new SSRConfiguration(config_path);
    instance_->init();
}

uint32_t SSRConfiguration::get_max_thread_num()
{
    return this->get_integer(SSR_GROUP_NAME, SSR_BASE_KEY_MAX_THREAD_NUM, MAX_THREAD_NUM_DEFAULT);
}

SSRStandardType SSRConfiguration::get_standard_type()
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

bool SSRConfiguration::set_standard_type(SSRStandardType standard_type)
{
    RETURN_VAL_IF_FALSE(standard_type < SSRStandardType::SSR_STANDARD_TYPE_LAST, false);

    // 如果设置的加固标准类型不存在对应的文件，则返回错误
    if ((standard_type == SSRStandardType::SSR_STANDARD_TYPE_SYSTEM && this->system_rs_.empty()) ||
        (standard_type == SSRStandardType::SSR_STANDARD_TYPE_CUSTOM && this->custom_rs_.empty()))
    {
        // error_code = SSRErrorCode::ERROR_CORE_RS_NOT_FOUND;
        return false;
    }

    return this->set_integer(SSR_GROUP_NAME, SSR_BASE_KEY_STANDARD_TYPE, int32_t(standard_type));
}

std::string SSRConfiguration::get_rs()
{
    switch (this->get_standard_type())
    {
    case SSRStandardType::SSR_STANDARD_TYPE_SYSTEM:
        return this->system_rs_;
    case SSRStandardType::SSR_STANDARD_TYPE_CUSTOM:
        return this->custom_rs_;
    default:
        return std::string();
    }
}

bool SSRConfiguration::set_custom_rs(const std::string& encrypted_rs, SSRErrorCode& error_code)
{
    // 判断自定义加固标准
    auto decrypted_rs = CryptoHelper::rsa_decrypt(RSA_PUBLIC_KEY_FILEPATH, encrypted_rs);
    if (decrypted_rs.empty())
    {
        error_code = SSRErrorCode::ERROR_CUSTOM_RS_DECRYPT_FAILED;
        return false;
    }

    try
    {
        Glib::file_set_contents(CUSTOM_RS_FILEPATH, encrypted_rs);
        // 更新自定义加固配置
        this->custom_rs_ = decrypted_rs;
        return true;
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }
    return false;
}

bool SSRConfiguration::set_custom_ra(const std::string& ra)
{
    try
    {
        Glib::file_set_contents(CUSTOM_RA_FILEPATH, ra);
        this->custom_ra_ = ra;
        return true;
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }
    return false;
}

void SSRConfiguration::init()
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

    this->load_rs_files();
    this->load_ra_files();
}

void SSRConfiguration::load_rs_files()
{
    KLOG_PROFILE("");

    // 加载加固标准
    this->system_rs_ = this->decrypt_file(SYSTEM_RS_FILEPATH);
    this->custom_rs_ = this->decrypt_file(CUSTOM_RS_FILEPATH);

    // KLOG_DEBUG("system rs: %s.", this->system_rs_.c_str());
    // KLOG_DEBUG("custom rs: %s.", this->custom_rs_.c_str());
}

void SSRConfiguration::load_ra_files()
{
    KLOG_PROFILE("");

    this->custom_ra_.clear();
    RETURN_IF_TRUE(!Glib::file_test(CUSTOM_RA_FILEPATH, Glib::FILE_TEST_IS_REGULAR));

    try
    {
        this->custom_ra_ = Glib::file_get_contents(CUSTOM_RA_FILEPATH);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }
}

std::string SSRConfiguration::decrypt_file(const std::string& filename)
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
        KLOG_WARNING("%s.", e.what());
    }
    return std::string();
}

int32_t SSRConfiguration::get_integer(const std::string& group_name, const std::string& key, int32_t default_value)
{
    int32_t retval = default_value;
    IGNORE_EXCEPTION(retval = this->configuration_.get_integer(group_name, key));
    return retval;
}

std::string SSRConfiguration::get_string(const std::string& group_name, const std::string& key)
{
    std::string retval;
    IGNORE_EXCEPTION(retval = this->configuration_.get_string(group_name, key));
    return retval;
}

std::string SSRConfiguration::get_datadir_filename(const std::string& group_name, const std::string& key)
{
    auto basename = this->get_string(group_name, key);
    RETURN_VAL_IF_TRUE(basename.empty(), std::string());
    return Glib::build_filename(SSR_INSTALL_DATADIR, basename);
}

bool SSRConfiguration::set_integer(const std::string& group_name, const std::string& key, int32_t value)
{
    this->configuration_.set_integer(SSR_GROUP_NAME, SSR_BASE_KEY_STANDARD_TYPE, value);
    return this->save_to_file();
}

bool SSRConfiguration::set_string(const std::string& group_name, const std::string& key, const std::string& value)
{
    this->configuration_.set_string(group_name, key, value);
    return this->save_to_file();
}

bool SSRConfiguration::save_to_file()
{
    try
    {
        return this->configuration_.save_to_file(this->config_path_);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
        return false;
    }
}

}  // namespace Kiran