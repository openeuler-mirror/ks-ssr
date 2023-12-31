/**
 * @file          /ks-ssr-manager/src/tool/config/kv.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

/* 
KV类型配置：
    每行是一个分割符分割的两列组成的键值(key-value)对。
    写入时首先判断文件是否已经存在key，如果存在则直接替换value，否则新增一行。
*/
namespace KS
{
namespace Config
{
class KV
{
public:
    /**
     * @brief 为一个配置文件创建ConfigPlain
     * @param {conf_path}  配置文件路径。
     * @param {kv_split_pattern}  分割键值对的正则表达式
     * @param {kv_join_str}  拼接键值对的字符串
     * @param {comment} 注释字符串
     * @return {} 
     */
    KV(const std::string &conf_path,
       const std::string &kv_split_pattern,
       const std::string &kv_join_str,
       const std::string &comment);

public:
    virtual ~KV();

    bool get(const std::string &key, std::string &value);
    // 如果key不存在则自动创建
    bool set(const std::string &key, const std::string &value);
    // 设置所有key值匹配项
    bool set_all(const std::string &key, const std::string &value);
    bool del(const std::string &key);

private:
    std::string conf_path_;
    std::string kv_split_pattern_;
    std::string kv_join_str_;
    std::string comment_;
};
}  // namespace Config
}  // namespace KS
