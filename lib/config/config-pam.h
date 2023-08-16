/**
 * @file          /kiran-sse-manager/lib/config/config-pam.h
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

/* 
pam类型配置：
    pam类型配置文件，以so为分割符号进行分割为两列，然后给左侧添加.so，保证完整性，行成整体的键值对(key-value)，
    然后对value，以空格为分隔符进行解析，得到单个参数内容，将其中两个数据中间出现操作符的时候，将三者组合，最后得到key 对应 参数组合的情况。
    写入时首先判断文件是否已经存在key，如果存在，在value组中查找参数，存在则直接替换，否则添加至末尾；否则暂不进行操作，pam文件中存在顺序问题。
*/
namespace Kiran
{
class ConfigPAM
{
public:
    virtual ~ConfigPAM();

    bool has_key(const std::string &key);
    std::string get_value(const std::string &key);
    std::vector<std::string>  get_all_value(const std::string &key);
    // 传入的key必须存在，如果value不存在，则可以创建   参数为非表达式
    bool set_value(const std::string &key, const std::string &value);
    //参数为表达式的情况， 不等式、等式等。
    bool set_value(const std::string &key, const std::string &value, const std::string &opera, const int num);

    // std::vector<std::shared_ptr<ConfigPAMLine>> get_lines(const std::string &so_name);
    // std::shared_ptr<ConfigPAMLine> get_line(const std::string &so_name, int32_t type, bool required);

    bool delete_key(const std::string &key);

public:
    static std::shared_ptr<ConfigPAM> create(const std::string &conf_path);

private:
    ConfigPAM(const std::string &conf_path);

    void init();

    void split_values();
    // 从文件读取数据
    bool read_from_file();
    // 准备写入文件，这里做一个延时写入操作
    void write_to_file_ready();
    // 将数据写入文件
    bool write_to_file();

    bool on_write_to_file_idle_cb();
    void on_config_file_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);

private:
    std::string conf_path_;

    static std::map<std::string, std::shared_ptr<ConfigPAM>> pams_;

    // 键值对（key-value pairs）
    std::map<std::string, std::string> kvs_;
    std::map<std::string, std::vector<std::string>> kv_;
    // 是否正在写入文件
    bool is_writing_;
    Glib::RefPtr<Gio::FileMonitor> config_file_monitor_;
    sigc::connection write_file_idle_id_;
};
}  // namespace Kiran
