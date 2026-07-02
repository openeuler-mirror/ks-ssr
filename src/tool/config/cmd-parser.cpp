/**
 * @file          /kiran-ssr-manager/src/tool/config/cmd-parser.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/tool/config/cmd-parser.h"
#include "src/tool/config/kv.h"
#include "src/tool/config/pam.h"
#include "src/tool/config/table.h"

namespace Kiran
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
    this->option_group_.add_entry(MiscUtils::create_option_entry("get", N_("Specify the key or rule to get value"), "KEY"), this->options_.get_key);
    this->option_group_.add_entry(MiscUtils::create_option_entry("set", N_("Specify the key or rule to set value"), "KEY"), this->options_.set_key);
    this->option_group_.add_entry(MiscUtils::create_option_entry("del", N_("Specify the key or rule to delete record"), "KEY"), this->options_.del_key);
    this->option_group_.add_entry(MiscUtils::create_option_entry("value", N_("Specify the set value")), this->options_.set_value);
    this->option_group_.add_entry(MiscUtils::create_option_entry("line-match-pattern", N_("Specify regular expression to match the line. If many lines is matched, then the first matched line is used only")), this->options_.line_match_pattern);
    this->option_group_.add_entry(MiscUtils::create_option_entry("split-pattern", N_("Specify regular expression to split line")), this->options_.split_pattern);
    this->option_group_.add_entry(MiscUtils::create_option_entry("join-str", N_("Specify string for joining fields to line")), this->options_.join_str);

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

    if (!Glib::file_test(this->options_.file_path, Glib::FILE_TEST_EXISTS))
    {
        fmt::print(stderr, _("The file {0} doesn't exist"), this->options_.file_path);
        return EXIT_FAILURE;
    }

    if (this->options_.type.empty())
    {
        fmt::print(stderr, _("No specify file type"));
        return EXIT_FAILURE;
    }

    switch (Kiran::shash(this->options_.type.c_str()))
    {
    case CONNECT(CONFIG_TYPE_KV, _hash):
        return this->process_kv();
    case CONNECT(CONFIG_TYPE_PAM, _hash):
        return this->process_pam();
    case CONNECT(CONFIG_TYPE_TABLE, _hash):
        return this->process_table();
    default:
        fmt::print(stderr, _("Unknown file type"));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int CmdParser::process_kv()
{
    auto kv = KV(this->options_.file_path, this->options_.split_pattern, this->options_.join_str);
    if (!this->options_.get_key.empty())
    {
        std::string value;
        if (!kv.get(this->options_.get_key, value))
        {
            fmt::print(stderr, _("Failed to get value for {0}"), this->options_.get_key);
            return EXIT_FAILURE;
        }

        fmt::print("{0}", value);
        return EXIT_SUCCESS;
    }

    if (!this->options_.set_key.empty())
    {
        if (!kv.set(this->options_.set_key, this->options_.set_value))
        {
            fmt::print(stderr, _("Failed to set value for {0}"), this->options_.set_key);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (!this->options_.del_key.empty())
    {
        if (!kv.del(this->options_.set_key))
        {
            fmt::print(stderr, _("Failed to delete value for {0}"), this->options_.del_key);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

int CmdParser::process_pam()
{
    auto pam = PAM(this->options_.file_path, this->options_.line_match_pattern);

    if (!this->options_.get_key.empty())
    {
        std::string value;
        if (!pam.get(this->options_.get_key, this->options_.split_pattern, value))
        {
            fmt::print(stderr, _("Failed to get value for {0}"), this->options_.get_key);
            return EXIT_FAILURE;
        }
        fmt::print("{0}", value);
        return EXIT_SUCCESS;
    }

    if (!this->options_.set_key.empty())
    {
        if (!pam.set(this->options_.set_key,
                     this->options_.split_pattern,
                     this->options_.set_value,
                     this->options_.join_str))
        {
            fmt::print(stderr, _("Failed to set value for {0}"), this->options_.set_key);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

int CmdParser::process_table()
{
    auto table = Table(this->options_.file_path, this->options_.split_pattern, this->options_.join_str);
    std::vector<std::pair<int32_t, std::string>> cols;
    std::function<bool(std::vector<std::string>)> pred = [&cols](std::vector<std::string> fields) -> bool {
        for (auto& col : cols)
        {
            // 列不存在则直接返回不匹配
            RETURN_VAL_IF_TRUE(col.first - 1 >= (int32_t)fields.size(), false);

            if (col.second != fields[col.first - 1])
            {
                return false;
            }
        }
        return true;
    };

    if (!this->options_.get_key.empty())
    {
        cols = this->str2cols(this->options_.get_key);
        std::string value;
        auto retval = table.get(pred, value);

        if (!retval)
        {
            fmt::print(stderr, _("Failed to get value for {0}"), this->options_.get_key);
            return EXIT_FAILURE;
        }

        fmt::print("{0}", value);
        return EXIT_SUCCESS;
    }

    if (!this->options_.set_key.empty())
    {
        cols = this->str2cols(this->options_.set_key);
        if (!table.set(this->options_.set_value, pred))
        {
            fmt::print(stderr, _("Failed to set value for {0}"), this->options_.set_key);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (!this->options_.del_key.empty())
    {
        cols = this->str2cols(this->options_.del_key);
        if (!table.del(pred))
        {
            fmt::print(stderr, _("Failed to delete record for rule {0}"), this->options_.del_key);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

std::vector<std::pair<int32_t, std::string>> CmdParser::str2cols(const std::string& str)
{
    std::vector<std::pair<int32_t, std::string>> retval;
    auto fields = StrUtils::split_with_char(str, ';');

    for (auto field : fields)
    {
        auto kv = StrUtils::split_with_char(field, '=');
        auto pos = field.find('=');
        if (pos != std::string::npos)
        {
            auto colunm_index = std::strtol(field.substr(0, pos).c_str(), nullptr, 0);
            retval.push_back(std::make_pair(int32_t(colunm_index), field.substr(pos + 1)));
        }
    }
    return retval;
}
}  // namespace Config

}  // namespace Kiran
