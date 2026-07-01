/**
 * @file          /kiran-sse-manager/lib/core/sse-configuration.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

namespace Kiran
{
class SSEConfiguration
{
public:
    SSEConfiguration(const std::string& config_path);
    virtual ~SSEConfiguration();

    static SSEConfiguration* get_instance() { return instance_; };

    static void global_init(const std::string& config_path);

    static void global_deinit() { delete instance_; };

    // 获取最大线程数
    uint32_t get_max_thread_num();
    // 获取标准类型
    SSEStandardType get_standard_type();
    // 设置标准类型
    bool set_standard_type(SSEStandardType standard_type);
    // 获取加固标准
    std::string get_rs();
    // 设置自定义加固标准
    bool set_custom_rs(const std::string& encrypted_rs, SSEErrorCode& error_code);
    // 获取加固参数
    std::string get_custom_ra() { return this->custom_ra_; };
    // 设置加固参数
    bool set_custom_ra(const std::string& ra);

private:
    //
    void init();

    // 加载加固标准文件
    void load_rs_files();
    // 加载加固参数文件
    void load_ra_files();

    // 解密文件并返回字符串
    std::string decrypt_file(const std::string& filename);

    int32_t get_integer(const std::string& group_name, const std::string& key, int32_t default_value = 0);
    std::string get_string(const std::string& group_name, const std::string& key);
    // 通过group_name和key获取basename，然后返回${datadir}/basename
    std::string get_datadir_filename(const std::string& group_name, const std::string& key);
    bool set_integer(const std::string& group_name, const std::string& key, int32_t value);
    bool set_string(const std::string& group_name, const std::string& key, const std::string& value);

    // 保存配置到文件
    bool save_to_file();

private:
    static SSEConfiguration* instance_;

    // 配置文件路径
    std::string config_path_;
    // 配置文件内容
    Glib::KeyFile configuration_;

    // 系统加固标准
    std::string system_rs_;
    // 用户自定义加固标准
    std::string custom_rs_;

    // 用户自定义加固参数
    std::string custom_ra_;
};
}  // namespace Kiran