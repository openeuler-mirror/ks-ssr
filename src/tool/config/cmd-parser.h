/**
 * @file          /ks-ssr-manager/src/tool/config/cmd-parser.h
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
struct CommandOptions
{
    Glib::ustring type;
    Glib::ustring file_path;
    Glib::ustring method;
    Glib::ustring key;
    Glib::ustring value;
    Glib::ustring line_match_pattern;
    Glib::ustring split_pattern;
    Glib::ustring join_str;
    Glib::ustring comment;
    Glib::ustring new_line;
    Glib::ustring next_line_match_pattern;
};

class CmdParser
{
public:
    CmdParser();
    virtual ~CmdParser(){};

    // 初始化
    void init();
    // 运行
    int run(int& argc, char**& argv);

private:
    int process_kv();
    int process_pam();
    int process_table();

    // 将字符串转化为列值列表，例如"1=aa;2=bb"->[(1,"aa"), (2, "bb")]
    std::vector<std::pair<int32_t, std::string>> str2cols(const std::string& str);

private:
    Glib::OptionContext option_context_;
    Glib::OptionGroup option_group_;

    CommandOptions options_;
};

}  // namespace Config

}  // namespace KS
