/**
 * @file          /ks-ssr-manager/src/tool/config/pam.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

namespace KS
{
namespace Config
{
class PAM
{
public:
    struct MatchLineInfo
    {
        std::string content;
        std::string match_line;
        int32_t match_pos;
        bool is_match_comment;
        MatchLineInfo() : match_pos(0), is_match_comment(false) {}
    };

    /**
     * @brief 创建PAM对象
     * @param {conf_path} PAM配置路径。
     * @param {line_match_pattern}  需要操作的行，如果有多个行都匹配，则随机选择一个
     * @param {fallback_line} 如果未找到匹配行，则使用传入的缺省行
     * @return {} 
     */
    PAM(const std::string &conf_path,
        const std::string &line_match_pattern);

    /**
     * @brief 获取匹配行中的键值对
     * @param {key}  指定键
     * @param {kv_split_pattern}  指定分割键值对的正则匹配，如果设置为空，则代表查询key是否存在
     * @param {value}  如果kv_split_pattern设置为空，则value表示key是否存在（被设置为"true"或者"false")，否则value为获取到的值。
     * @return {} 如果出错则返回false,否则返回true
     */
    bool get_value(const std::string &key, const std::string &kv_split_pattern, std::string &value);

    bool set_value(const std::string &key,
                   const std::string &kv_split_pattern,
                   const std::string &value,
                   const std::string &kv_join_str);

    bool del_value(const std::string &key, const std::string &kv_split_pattern);

    // 如果没有找到匹配行，则将fallback_line添加到末尾
    bool add_line(const std::string &fallback_line, const std::string &next_line_match_regex);
    // 删除匹配行
    bool del_line();
    // 获取匹配行
    bool get_line(std::string &line);

private:
    PAM::MatchLineInfo get_match_line();
    PAM::MatchLineInfo add_behind(const std::string &fallback_line, const std::string &next_line_match_regex);

    // 将内容写入文件
    bool write_to_file(const std::string &content);

    // 最后一个字符是否为空白字符
    bool is_whitespace_in_tail(const std::string &str);

private:
    std::string conf_path_;
    std::string line_match_pattern_;
};
}  // namespace Config

}  // namespace KS
