/**
 * @file          /kiran-ssr-manager/src/tool/config/pam.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

namespace Kiran
{
namespace Config
{
class PAM
{
public:
    PAM(const std::string &conf_path,
        const std::string &line_match_pattern);

    /**
     * @brief 获取匹配行中的键值对
     * @param {key}  指定键
     * @param {kv_split_pattern}  指定分割键值对的正则匹配，如果设置为空，则代表查询key是否存在
     * @param {value}  如果kv_split_pattern设置为空，则value表示key是否存在（被设置为"true"或者"false")，否则value为获取到的值。
     * @return {} 如果出错则返回false,否则返回true
     */
    bool get(const std::string &key, const std::string &kv_split_pattern, std::string &value);

    bool set(const std::string &key,
             const std::string &kv_split_pattern,
             const std::string &value,
             const std::string &kv_join_str);
    bool del(const std::string &key, const std::string &kv_split_regex);

private:
    // 最后一个字符是否为空白字符
    bool is_whitespace_in_tail(const std::string &str);

private:
    std::string conf_path_;
    std::string line_match_pattern_;
};
}  // namespace Config

}  // namespace Kiran
