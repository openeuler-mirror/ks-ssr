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
class ConfigPlain
{
public:
    virtual ~ConfigPlain();

    bool has_key(const std::string &key);
    std::string get_value(const std::string &key);
    // 如果key不存在则自动创建
    bool update_value(const std::string &key, const std::string &value);
    bool delete_key(const std::string &key);

    // 提供简单的类型判断和转换函数
    // 是否可以作为bool类型处理
    bool is_bool(const std::string &key);
    // 如果类型错误或key不存在则返回false
    bool get_bool(const std::string &key);
    // 是否可以作为整形处理
    bool is_integer(const std::string &key);
    // 如果类型错误或key不存在则返回0
    int64_t get_integer(const std::string &key);
    // 是否可以作为浮点类型处理
    bool is_double(const std::string &key);
    // 如果类型错误或key不存在则返回0
    double get_double(const std::string &key);

public:
    static std::shared_ptr<ConfigPlain> create(const std::string &conf_path);

private:
    ConfigPlain(const std::string &conf_path);

    void init();

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

    static std::map<std::string, std::shared_ptr<ConfigPlain>> plains_;

    // 键值对（key-value pairs）
    std::map<std::string, std::string> kvs_;
    // 是否正在写入文件
    bool is_writing_;
    Glib::RefPtr<Gio::FileMonitor> config_file_monitor_;
    sigc::connection write_file_idle_id_;
};
}  // namespace Kiran
