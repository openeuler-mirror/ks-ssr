/**
 * @file          /kiran-ssr-manager/src/tool/config/cmd-parser.h
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
struct CommandOptions
{
    Glib::ustring type;
    Glib::ustring file_path;
    Glib::ustring get_key;
    Glib::ustring set_key;
    Glib::ustring set_value;
    Glib::ustring line_match_pattern;
    Glib::ustring kv_split_pattern;
    Glib::ustring kv_join_str;
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
    int process_plain();
    int process_pam();

private:
    Glib::OptionContext option_context_;
    Glib::OptionGroup option_group_;

    CommandOptions options_;
};

}  // namespace Config

}  // namespace Kiran
