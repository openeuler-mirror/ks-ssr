/**
 * @file          /ks-ssr-manager/src/tool/config/cmd-parser.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/tool/config/cmd-parser.h"
#include "src/tool/config/kv.h"
#include "src/tool/config/pam.h"
#include "src/tool/config/table.h"

namespace KS
{
namespace Config
{
#define CONFIG_TYPE_KV "KV"
#define CONFIG_TYPE_PAM "PAM"
#define CONFIG_TYPE_TABLE "TABLE"

CmdParser::CmdParser() : option_context_(N_("FILE")),
                         option_group_("config", "config options")
{
}

void CmdParser::init()
{
    this->option_group_.add_entry(MiscUtils::create_option_entry("type", N_("The configuration file type"), CONFIG_TYPE_KV "|" CONFIG_TYPE_PAM "|" CONFIG_TYPE_TABLE), this->options_.type);
    this->option_group_.add_entry(MiscUtils::create_option_entry("method", N_("The Operation method"), "GETVAL|SETVAL|SETVALALL|DELVAL|GETLINE|SETLINE|DELLINE"), this->options_.method);
    this->option_group_.add_entry(MiscUtils::create_option_entry("key", N_("Specify the key or rule to get value"), "KEY"), this->options_.key);
    this->option_group_.add_entry(MiscUtils::create_option_entry("value", N_("Specify the set value")), this->options_.value);
    this->option_group_.add_entry(MiscUtils::create_option_entry("line-match-pattern", N_("Specify regular expression to match the line. If many lines is matched, then the first matched line is used only")), this->options_.line_match_pattern);
    this->option_group_.add_entry(MiscUtils::create_option_entry("split-pattern", N_("Specify regular expression to split line")), this->options_.split_pattern);
    this->option_group_.add_entry(MiscUtils::create_option_entry("join-str", N_("Specify string for joining fields to line")), this->options_.join_str);
    this->option_group_.add_entry(MiscUtils::create_option_entry("comment", N_("Specify comment string")), this->options_.comment);
    this->option_group_.add_entry(MiscUtils::create_option_entry("new-line", N_("Add new line when the speficied line pattern is dismatch in PAM")), this->options_.new_line);
    this->option_group_.add_entry(MiscUtils::create_option_entry("next-line-match-pattern", N_("Specifies a regular expression to match the next row of the inserted row. If multiple rows are matched, the value is used by the first matched row")), this->options_.next_line_match_pattern);

    this->option_group_.set_translation_domain(PROJECT_NAME);
    this->option_context_.set_translation_domain(PROJECT_NAME);
    this->option_context_.set_main_group(this->option_group_);
}

int CmdParser::run(int& argc, char**& argv)
{
    try
    {
        this->option_context_.parse(argc, argv);
    }
    catch (const Glib::Exception& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        fmt::print(stderr, _("Failed to parse command arguments"));
        return EXIT_FAILURE;
    }

    if (argc != 2)
    {
        fmt::print(stderr, _("The file path is not specified"));
        return EXIT_FAILURE;
    }
    this->options_.file_path = POINTER_TO_STRING(argv[1]);

    // if (!Glib::file_test(this->options_.file_path, Glib::FILE_TEST_EXISTS))
    // {
    //     fmt::print(stderr, _("The file {0} doesn't exist"), this->options_.file_path);
    //     return EXIT_FAILURE;
    // }

    if (this->options_.type.empty())
    {
        fmt::print(stderr, _("No specify file type"));
        return EXIT_FAILURE;
    }

    if (this->options_.type == CONFIG_TYPE_KV)
    {
        return this->process_kv();
    }
    else if (this->options_.type == CONFIG_TYPE_PAM)
    {
        return this->process_pam();
    }
    else if (this->options_.type == CONFIG_TYPE_TABLE)
    {
        return this->process_table();
    }
    else
    {
        fmt::print(stderr, _("Unknown file type"));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int CmdParser::process_kv()
{
    auto kv = KV(this->options_.file_path, this->options_.split_pattern, this->options_.join_str, this->options_.comment);
    bool retval = false;

    if (this->options_.method == "GETVAL")
    {
        std::string value;
        retval = kv.get(this->options_.key, value);
        if (retval)
        {
            fmt::print("{0}", value);
        }
    }
    else if (this->options_.method == "SETVAL")
    {
        retval = kv.set(this->options_.key, this->options_.value);
    }
    else if (this->options_.method == "SETVALALL")
    {
        retval = kv.set_all(this->options_.key, this->options_.value);
    }
    else if (this->options_.method == "DELVAL")
    {
        retval = kv.del(this->options_.key);
    }

    if (!retval)
    {
        fmt::print(stderr, _("Exec method {0} failed"), this->options_.method.raw());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int CmdParser::process_pam()
{
    auto pam = PAM(this->options_.file_path, this->options_.line_match_pattern);

    bool retval = false;

    if (this->options_.method == "GETVAL")
    {
        std::string value;
        retval = pam.get_value(this->options_.key, this->options_.split_pattern, value);
        if (retval)
        {
            fmt::print("{0}", value);
        }
    }
    else if (this->options_.method == "SETVAL")
    {
        retval = pam.set_value(this->options_.key, this->options_.split_pattern, this->options_.value, this->options_.join_str);
    }
    else if (this->options_.method == "DELVAL")
    {
        retval = pam.del_value(this->options_.key, this->options_.split_pattern);
    }
    else if (this->options_.method == "SETLINE")
    {
        retval = pam.add_line(this->options_.new_line, this->options_.next_line_match_pattern);
    }
    else if (this->options_.method == "DELLINE")
    {
        retval = pam.del_line();
    }
    else if (this->options_.method == "GETLINE")
    {
        std::string line;
        retval = pam.get_line(line);
        if (retval)
        {
            fmt::print("{0}", line);
        }
    }

    if (!retval)
    {
        fmt::print(stderr, _("Exec method {0} failed"), this->options_.method.raw());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int CmdParser::process_table()
{
    auto table = Table(this->options_.file_path, this->options_.split_pattern, this->options_.join_str);
    std::vector<std::pair<int32_t, std::string>> cols = this->str2cols(this->options_.key);
    std::function<bool(std::vector<std::string>)> pred = [&cols](std::vector<std::string> fields) -> bool {
        for (auto iter = cols.begin(); iter != cols.end(); ++iter)
        {
            // 列不存在则直接返回不匹配
            RETURN_VAL_IF_TRUE(iter->first - 1 >= (int32_t)fields.size(), false);

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
        std::string value;
        retval = table.get(pred, value);
        if (retval)
        {
            fmt::print("{0}", value);
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
        fmt::print(stderr, _("Exec method {0} failed"), this->options_.method.raw());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

std::vector<std::pair<int32_t, std::string>> CmdParser::str2cols(const std::string& str)
{
    std::vector<std::pair<int32_t, std::string>> retval;
    auto fields = StrUtils::split_with_char(str, ';');

    for (auto iter = fields.begin(); iter != fields.end(); ++iter)
    {
        auto& field = (*iter);
        auto kv = StrUtils::split_with_char(field, '=');
        auto pos = field.find('=');
        if (pos != std::string::npos)
        {
            auto colunm_index = std::strtol(field.substr(0, pos).c_str(), NULL, 0);
            retval.push_back(std::make_pair(int32_t(colunm_index), field.substr(pos + 1)));
        }
    }
    return retval;
}
}  // namespace Config

}  // namespace KS
