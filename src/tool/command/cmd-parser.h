/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
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
    int processKv();
    int processPam();
    int processTable();

    // 将字符串转化为列值列表，例如"1=aa;2=bb"->[(1,"aa"), (2, "bb")]
    std::vector<std::pair<int32_t, std::string>> str2cols(const std::string& str);

private:
    Glib::OptionContext option_context_;
    Glib::OptionGroup option_group_;

    CommandOptions options_;
};

}  // namespace Config

}  // namespace KS
