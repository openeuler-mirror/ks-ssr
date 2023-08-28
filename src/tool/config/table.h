/**
 * @file          /ks-ssr-manager/src/tool/config/table.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

/* TABLE类型配置： 每行是一个分割符分割的多列数据，每行的列数量可以不相同 */

namespace KS
{
namespace Config
{
class Table
{
public:
    /**
     * @brief 为一个配置文件创建ConfigPlain
     * @param {conf_path}  配置文件路径。
     * @param {split_pattern}  分割列的正则表达式，如果为空，表示每一行一个key，没有value
     * @param {join_str}  拼接列的字符串
     * @return {}
     */
    Table(const std::string &conf_path,
          const std::string &split_pattern,
          const std::string &join_str);

public:
    virtual ~Table();

    // 返回第一个匹配pred条件的行
    bool get(std::function<bool(std::vector<std::string>)> pred, std::string &value);
    // 替换第一个满足pred条件的行，如果不存在则插入新行
    bool set(const std::string &newline, std::function<bool(std::vector<std::string>)> pred);
    // 删除所有满足pred的行
    bool del(std::function<bool(std::vector<std::string>)> pred);

private:
    std::string conf_path_;
    std::string split_pattern_;
    std::string join_str_;
};
}  // namespace Config
}  // namespace KS
