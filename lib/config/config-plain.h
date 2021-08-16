/**
 * @file          /kiran-sse-manager/lib/config/config-plain.h
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
    bool set_value(const std::string &key, const std::string &value);
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
    /**
     * @brief 为一个配置文件创建ConfigPlain，如果指定的配置之前已经创建过，则直接返回之前创建的配置对象
     * @param {conf_path}  配置文件路径。
     * @param {delimiter_pattern}  列分割字符串的正则匹配模式
     * @param {insert_delimiter}  在插入新行时使用的分割字符串
     * @return {} 返回ConfigPlain对象
     */
    static std::shared_ptr<ConfigPlain> create(const std::string &conf_path,
                                               std::string delimiter_pattern = "\\s+",
                                               std::string insert_delimiter = "\t");

private:
    ConfigPlain(const std::string &conf_path,
                const std::string &delimiter_pattern,
                const std::string &insert_delimiter);

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
    std::string delimiter_pattern_;
    std::string insert_delimiter_;

    static std::map<std::string, std::shared_ptr<ConfigPlain>> plains_;

    // 键值对（key-value pairs）
    std::map<std::string, std::string> kvs_;
    // 是否正在写入文件
    bool is_writing_;
    Glib::RefPtr<Gio::FileMonitor> config_file_monitor_;
    sigc::connection write_file_idle_id_;
};
}  // namespace Kiran
