/**
 * @file          /kiran-ssr-manager/src/tool/config/cmd-parser.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/tool/config/cmd-parser.h"
#include "src/tool/config/pam.h"
#include "src/tool/config/plain.h"

namespace Kiran
{
namespace Config
{
#define CONFIG_TYPE_PLAIN "PLAIN"
#define CONFIG_TYPE_PAM "PAM"

CmdParser::CmdParser() : option_context_(N_("FILE")),
                         option_group_("config", "config options")
{
}

void CmdParser::init()
{
    this->option_group_.add_entry(MiscUtils::create_option_entry("type", N_("The configuration file type"), CONFIG_TYPE_PLAIN "|" CONFIG_TYPE_PAM), this->options_.type);
    this->option_group_.add_entry(MiscUtils::create_option_entry("get", N_("Specify the key to get value"), "KEY"), this->options_.get_key);
    this->option_group_.add_entry(MiscUtils::create_option_entry("set", N_("Specify the key to set value"), "KEY"), this->options_.set_key);
    this->option_group_.add_entry(MiscUtils::create_option_entry("value", N_("Specify the set value")), this->options_.set_value);
    this->option_group_.add_entry(MiscUtils::create_option_entry("line-match-pattern", N_("Specify regular expression to match the line. If many lines is matched, then the first matched line is used only")), this->options_.line_match_pattern);
    this->option_group_.add_entry(MiscUtils::create_option_entry("kv-split-pattern", N_("Specify regular expression to split key-value pairs")), this->options_.kv_split_pattern);
    this->option_group_.add_entry(MiscUtils::create_option_entry("kv-join-str", N_("Specify string to join key-value pairs")), this->options_.kv_join_str);

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
    case CONNECT(CONFIG_TYPE_PLAIN, _hash):
        return this->process_plain();
    case CONNECT(CONFIG_TYPE_PAM, _hash):
        return this->process_pam();
    default:
        fmt::print(stderr, _("Unknown file type"));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int CmdParser::process_plain()
{
    auto plain = Plain(this->options_.file_path, this->options_.kv_split_pattern, this->options_.kv_join_str);
    if (!this->options_.get_key.empty())
    {
        std::string value;
        if (!plain.get(this->options_.get_key, value))
        {
            fmt::print(stderr, _("Failed to get value for {0}"), this->options_.get_key);
            return EXIT_FAILURE;
        }

        fmt::print("{0}", value);
        return EXIT_SUCCESS;
    }

    if (!this->options_.set_key.empty())
    {
        if (!plain.set(this->options_.set_key, this->options_.set_value))
        {
            fmt::print(stderr, _("Failed to set value for {0}"), this->options_.set_key);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int CmdParser::process_pam()
{
    auto pam = PAM(this->options_.file_path, this->options_.line_match_pattern);

    if (!this->options_.get_key.empty())
    {
        std::string value;
        if (!pam.get(this->options_.get_key, this->options_.kv_split_pattern, value))
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
                     this->options_.kv_split_pattern,
                     this->options_.set_value,
                     this->options_.kv_join_str))
        {
            fmt::print(stderr, _("Failed to set value for {0}"), this->options_.set_key);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
}  // namespace Config

}  // namespace Kiran
