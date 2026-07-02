/**
 * @file          /ks-ssr-manager/src/daemon/configuration.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "src/daemon/ssr-protocol.hxx"

namespace KS
{
namespace Daemon
{
class Configuration
{
public:
    Configuration(const std::string& config_path);
    virtual ~Configuration();

    static Configuration* get_instance() { return instance_; };

    static void global_init(const std::string& config_path);

    static void global_deinit() { delete instance_; };

    // 获取最大线程数
    uint32_t get_max_thread_num();
    // 获取标准类型
    SSRStandardType get_standard_type();
    // 设置标准类型
    bool set_standard_type(SSRStandardType standard_type);
    // 获取加固策略类型
    SSRStrategyType get_strategy_type();
    // 设置加固策略类型
    bool set_strategy_type(SSRStrategyType strategy_type);
    // 检测导入ra文件是否正确
    bool check_ra_strategy();
    // 前台复选框勾选调用，checkbox后台默认为false
    void set_ra_checkbox(const std::string &name, const bool &status);
    // 获取加固标准
    std::shared_ptr<Protocol::RS> get_rs() { return this->rs_; }
    // 设置自定义加固标准
    bool set_custom_rs(const std::string& encrypted_rs, SSRErrorCode& error_code);
    // 设置加固参数
    bool set_custom_ra(const Protocol::Reinforcement& rs_reinforcement);
    // 删除加固项的自定义参数
    void del_custom_ra(const std::string& name);
    void del_all_custom_ra();


    // 加载历史加固参数文件
    std::shared_ptr<Protocol::ReinforcementHistory> read_rh_from_file(const std::string path);
    // 写历史加固参数文件
    bool write_rh_to_file(std::shared_ptr<Protocol::ReinforcementHistory> rh, const std::string path);
    // 设置历史加固参数
    bool set_custom_rh(const Protocol::Reinforcement& rs_reinforcement, const std::string path);

    SSRResourceMonitor get_resource_monitor_status();
    bool set_resource_monitor_status(SSRResourceMonitor resource_monitor);

    // 加固标准发生变化
    sigc::signal<void> signal_rs_changed() { return this->rs_changed_; };

private:
    //
    void init();

    // 重新加载加固标准，这里会发送变化的信号
    void reload_rs();
    void load_rs();
    // 修改加固参数，重载加固项
    void reload_strategy();
    // 加载加固标准文件(不变化的部分)
    std::shared_ptr<Protocol::RS> get_fixed_rs();
    // 加载加固参数文件
    std::shared_ptr<Protocol::RA> read_ra_from_file();
    // 写加固参数文件
    bool write_ra_to_file(std::shared_ptr<Protocol::RA> ra);

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
    static Configuration* instance_;

    // 配置文件路径
    std::string config_path_;
    // 配置文件内容
    Glib::KeyFile configuration_;

    // 加固标准和自定义加固参数的混合
    std::shared_ptr<Protocol::RS> rs_;

    sigc::signal<void> rs_changed_;
};
}  // namespace Daemon
}  // namespace KS
