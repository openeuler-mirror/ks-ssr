/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-configuration.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "src/daemon/ssr-protocol.hxx"

namespace Kiran
{
class SSRConfiguration
{
public:
    SSRConfiguration(const std::string& config_path);
    virtual ~SSRConfiguration();

    static SSRConfiguration* get_instance() { return instance_; };

    static void global_init(const std::string& config_path);

    static void global_deinit() { delete instance_; };

    // 获取最大线程数
    uint32_t get_max_thread_num();
    // 获取标准类型
    SSRStandardType get_standard_type();
    // 设置标准类型
    bool set_standard_type(SSRStandardType standard_type);
    // 获取加固标准
    std::shared_ptr<Protocol::RS> get_rs() { return this->rs_; }
    // 设置自定义加固标准
    bool set_custom_rs(const std::string& encrypted_rs, SSRErrorCode& error_code);
    // 设置加固参数
    bool set_custom_ra(const Protocol::Reinforcement& rs_reinforcement);

    // 加固标准发生变化
    sigc::signal<void> signal_rs_changed() { return this->rs_changed_; };

private:
    //
    void init();

    // 重新加载加固标准，这里会发送变化的信号
    void reload_rs();
    void load_rs();
    // 加载加固标准文件(不变化的部分)
    std::shared_ptr<Protocol::RS> get_fixed_rs();
    // 加载加固参数文件
    std::shared_ptr<Protocol::RA> get_ra();

    void join_reinforcement(Protocol::Reinforcement& to_r, const Protocol::Reinforcement& from_r);

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
    static SSRConfiguration* instance_;

    // 配置文件路径
    std::string config_path_;
    // 配置文件内容
    Glib::KeyFile configuration_;

    // 加固标准和自定义加固参数的混合
    std::shared_ptr<Protocol::RS> rs_;

    sigc::signal<void> rs_changed_;
};
}  // namespace Kiran