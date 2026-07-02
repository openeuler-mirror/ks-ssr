/**
 * @file          /kiran-ssr-manager/src/tool/config/plain.h
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
namespace Config
{
class Plain
{
public:
    /**
     * @brief 为一个配置文件创建ConfigPlain
     * @param {conf_path}  配置文件路径。
     * @param {kv_split_pattern}  分割键值对的正则表达式
     * @param {kv_join_str}  拼接键值对的字符串
     * @return {} 返回ConfigPlain对象
     */
    Plain(const std::string &conf_path,
          const std::string &kv_split_pattern,
          const std::string &kv_join_str);

public:
    virtual ~Plain();

    bool get(const std::string &key, std::string &value);
    // 如果key不存在则自动创建
    bool set(const std::string &key, const std::string &value);
    bool del(const std::string &key);

private:
    std::string conf_path_;
    std::string kv_split_pattern_;
    std::string kv_join_str_;
};
}  // namespace Config
}  // namespace Kiran
