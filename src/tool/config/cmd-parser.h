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

#include <QCommandLineParser>
#include <QPair>
#include "lib/base/base.h"

namespace KS
{
namespace Config
{
struct CommandOptions
{
    QString type;
    QString file_path;
    QString method;
    QString key;
    QString value;
    QString line_match_pattern;
    QString split_pattern;
    QString join_str;
    QString comment;
    QString new_line;
    QString next_line_match_pattern;
};

class CmdParser
{
public:
    CmdParser();
    virtual ~CmdParser(){};

    // 初始化
    void init();
    // 运行
    int run(int argc, char** argv, QCoreApplication& a);

private:
    int processKv();
    int processPam();
    int processTable();

    // 将字符串转化为列值列表，例如"1=aa;2=bb"->[(1,"aa"), (2, "bb")]
    QVector<QPair<int32_t, QString>> str2cols(const QString& str);

private:
    CommandOptions options_;
    QCommandLineParser parser;
};

}  // namespace Config

}  // namespace KS
