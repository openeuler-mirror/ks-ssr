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

#include "cmd-parser.h"
#include <QFileInfo>
#include <QString>
#include <QTranslator>
#include <iostream>
#include "kv.h"
#include "pam.h"
#include "table.h"

using std::cerr;
using std::cout;
using std::endl;

namespace KS
{
namespace Config
{
#define CONFIG_TYPE_KV "KV"
#define CONFIG_TYPE_PAM "PAM"
#define CONFIG_TYPE_TABLE "TABLE"

CmdParser::CmdParser()
{
}

void CmdParser::init()
{
    this->parser.addHelpOption();
    this->parser.addVersionOption();
    this->parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    this->parser.addOption({"type",
                            QObject::tr("The configuration file type"),
                            CONFIG_TYPE_KV "|" CONFIG_TYPE_PAM "|" CONFIG_TYPE_TABLE});
    this->parser.addOption({"method",
                            QObject::tr("The Operation method"),
                            "GETVAL|SETVAL|SETVALALL|DELVAL|GETLINE|SETLINE|DELLINE"});
    this->parser.addOption({"key",
                            QObject::tr("Specify the key or rule to get value"),
                            "KEY"});
    this->parser.addOption({"value",
                            QObject::tr("Specify the set value"),
                            "VALUE"});
    this->parser.addOption({"line-match-pattern",
                            QObject::tr("Specify regular expression to match the line. If many lines is matched, then the first matched line is used only"),
                            "PATTERN"});
    this->parser.addOption({"split-pattern",
                            QObject::tr("Specify regular expression to split line"),
                            "PATTERN"});
    this->parser.addOption({"join-str",
                            QObject::tr("Specify string for joining fields to line"),
                            "STR"});
    this->parser.addOption({"comment",
                            QObject::tr("Specify comment string"),
                            "COMMENT"});
    this->parser.addOption({"new-line",
                            QObject::tr("Add new line when the speficied line pattern is dismatch in PAM"),
                            "NEW-LINE"});
    this->parser.addOption({"next-line-match-pattern",
                            QObject::tr("Specifies a regular expression to match the next row of the inserted row. If multiple rows are matched, the value is used by the first matched row"),
                            "PATTERN"});
    this->parser.addPositionalArgument("File-Path", QObject::tr("the configuration's path"));
}

int CmdParser::run(int argc, char** argv, QCoreApplication& a)
{
    this->parser.process(a);
    if (this->parser.positionalArguments().isEmpty())
    {
        std::cout << QObject::tr("The file path is not specified").toStdString() << std::endl;
        return EXIT_FAILURE;
    }
    auto file_path = this->parser.positionalArguments().first();
    this->options_ = {
        this->parser.value("type"),
        file_path,
        this->parser.value("method"),
        this->parser.value("key"),
        this->parser.value("value"),
        this->parser.value("line-match-pattern"),
        this->parser.value("split-pattern"),
        this->parser.value("join-str"),
        this->parser.value("comment"),
        this->parser.value("new-line"),
        this->parser.value("next-line-match-pattern")};

    if (this->options_.type.isEmpty())
    {
        cerr << QObject::tr("No specify file type").toStdString() << endl;
        return EXIT_FAILURE;
    }

    if (this->options_.type == CONFIG_TYPE_KV)
    {
        return this->processKv();
    }
    else if (this->options_.type == CONFIG_TYPE_PAM)
    {
        return this->processPam();
    }
    else if (this->options_.type == CONFIG_TYPE_TABLE)
    {
        return this->processTable();
    }
    else
    {
        cerr << QObject::tr("Unknown file type").toStdString();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int CmdParser::processKv()
{
    auto kv = KV(this->options_.file_path, this->options_.split_pattern, this->options_.join_str, this->options_.comment);
    bool retval = false;

    if (this->options_.method == "GETVAL")
    {
        QString value;
        retval = kv.get(this->options_.key, value);
        if (retval)
        {
            cout << value.toStdString() << endl;
        }
    }
    else if (this->options_.method == "SETVAL")
    {
        retval = kv.set(this->options_.key, this->options_.value);
    }
    else if (this->options_.method == "SETVALALL")
    {
        retval = kv.setAll(this->options_.key, this->options_.value);
    }
    else if (this->options_.method == "DELVAL")
    {
        retval = kv.del(this->options_.key);
    }

    if (!retval)
    {
        cout << QObject::tr("Exec method %1 failed").arg(this->options_.method).toStdString();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int CmdParser::processPam()
{
    auto pam = PAM(this->options_.file_path, this->options_.line_match_pattern);

    bool retval = false;

    if (this->options_.method == "GETVAL")
    {
        QString value;
        retval = pam.getValue(this->options_.key, this->options_.split_pattern, value);
        if (retval)
        {
            cerr << value.toStdString() << endl;
        }
    }
    else if (this->options_.method == "SETVAL")
    {
        retval = pam.setValue(this->options_.key, this->options_.split_pattern, this->options_.value, this->options_.join_str);
    }
    else if (this->options_.method == "DELVAL")
    {
        retval = pam.delValue(this->options_.key, this->options_.split_pattern);
    }
    else if (this->options_.method == "SETLINE")
    {
        retval = pam.addLine(this->options_.new_line, this->options_.next_line_match_pattern);
    }
    else if (this->options_.method == "DELLINE")
    {
        retval = pam.delLine();
    }
    else if (this->options_.method == "GETLINE")
    {
        QString line;
        retval = pam.getLine(line);
        if (retval)
        {
            cout << line.toStdString() << endl;
        }
    }

    if (!retval)
    {
        cout << QObject::tr("Exec method %1 failed").arg(this->options_.method).toStdString() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int CmdParser::processTable()
{
    auto table = Table(this->options_.file_path, this->options_.split_pattern, this->options_.join_str);
    QVector<QPair<int32_t, QString>> cols = this->str2cols(this->options_.key);
    std::function<bool(QVector<QString>)> pred = [&cols](QVector<QString> fields) -> bool {
        for (auto iter = cols.begin(); iter != cols.end(); ++iter)
        {
            // 列不存在则直接返回不匹配
            RETURN_VAL_IF_TRUE(iter->first - 1 >= fields.size(), false);

            if (iter->second != fields[iter->first - 1])
            {
                return false;
            }
        }
        return true;
    };

    bool retval = false;

    if (this->options_.method == "GETVAL")
    {
        QString value;
        retval = table.get(pred, value);
        if (retval)
        {
            cout << value.toStdString() << endl;
        }
    }
    else if (this->options_.method == "SETVAL")
    {
        retval = table.set(this->options_.value, pred);
    }
    else if (this->options_.method == "DELVAL")
    {
        retval = table.del(pred);
    }

    if (!retval)
    {
        cerr << QObject::tr("Exec method %1 failed").arg(this->options_.method).toStdString() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

QVector<QPair<int32_t, QString>> CmdParser::str2cols(const QString& str)
{
    QVector<QPair<int32_t, QString>> retval;
    auto fields = StrUtils::splitWithChar(str, ';');

    for (auto iter = fields.begin(); iter != fields.end(); ++iter)
    {
        auto& field = (*iter);
        auto kv = StrUtils::splitWithChar(field, '=');
        auto pos = field.indexOf('=');
        if (pos != -1)
        {
            auto colunm_index = field.mid(0, pos).toLong();
            retval.push_back(qMakePair(int32_t(colunm_index), field.mid(pos + 1)));
        }
    }
    return retval;
}
}  // namespace Config

}  // namespace KS
