/**
 * @file          /kiran-sse-manager/lib/base/base.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

/* 
plain类型配置：
    没有任何固定格式的配置文件，读取时只保留由空白字符分割为两列的非注释行，该行的两列组成键值(key-value)对。
    写入时首先判断文件是否已经存在key，如果存在则直接替换value，否则新增一行。
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

    static std::map<std::string, std::shared_ptr<ConfigPAM>> plains_;

    // 键值对（key-value pairs）
    std::map<std::string, std::string> kvs_;
    std::map<std::string, std::vector<std::string>> kv_;
    // 是否正在写入文件
    bool is_writing_;
    Glib::RefPtr<Gio::FileMonitor> config_file_monitor_;
    sigc::connection write_file_idle_id_;
};
}  // namespace Kiran
