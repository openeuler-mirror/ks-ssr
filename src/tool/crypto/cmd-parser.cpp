/**
 * @file          /ks-ssr-manager/src/tool/crypto/cmd-parser.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/tool/crypto/cmd-parser.h"
#include <glib/gi18n.h>

namespace KS
{
namespace Crypto
{
#define SSR_RSA_LENGTH 1024

CmdParser::CmdParser() : option_group_(PROJECT_NAME, "group options")
{
}

void CmdParser::init()
{
    this->option_group_.add_entry(MiscUtils::create_option_entry("version", N_("Output version infomation and exit.")),
                                  this->options_.show_version);

    this->option_group_.add_entry(MiscUtils::create_option_entry("generate-rsa-key", N_("Generate public and private keys for RSA.")),
                                  this->options_.generate_rsa_key);

    this->option_group_.add_entry(MiscUtils::create_option_entry("decrypt-file", N_("Decrypt a file.")),
                                  this->options_.decrypted_file);

    this->option_group_.add_entry(MiscUtils::create_option_entry("encrypt-file", N_("Encrypt a file.")),
                                  this->options_.encrypted_file);

    this->option_group_.add_entry(MiscUtils::create_option_entry("public-key", N_("RSA public file path.")),
                                  this->options_.public_filename);

    this->option_group_.add_entry(MiscUtils::create_option_entry("private-key", N_("RSA private file path.")),
                                  this->options_.private_filename);

    this->option_group_.add_entry(MiscUtils::create_option_entry("output-file", N_("Output file path.")),
                                  this->options_.output_filename);

    this->option_group_.set_translation_domain(PROJECT_NAME);
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
        return EXIT_FAILURE;
    }

    if (this->options_.generate_rsa_key)
    {
        KS::CryptoHelper::generate_rsa_key(SSR_RSA_LENGTH, "ssr-public.key", "ssr-private.key");
    }
    else if (!this->options_.decrypted_file.empty())
    {
        RETURN_VAL_IF_FALSE(this->process_decrypt_file(), EXIT_FAILURE);
    }
    else if (!this->options_.encrypted_file.empty())
    {
        RETURN_VAL_IF_FALSE(this->process_encrypt_file(), EXIT_FAILURE);
    }
    else
    {
        // 默认显示版本
        auto program_name = Glib::path_get_basename(argv[0]);
        fmt::print("%s: %s\n", program_name.c_str(), PROJECT_VERSION);
    }
    return EXIT_SUCCESS;
}

bool CmdParser::process_decrypt_file()
{
    if (this->options_.public_filename.empty())
    {
        fmt::print(stderr, "RSA public file isn't provided.");
        return false;
    }

    if (this->options_.output_filename.empty())
    {
        fmt::print(stderr, "Output file isn't provided.");
        return false;
    }

    try
    {
        auto message = Glib::file_get_contents(this->options_.decrypted_file);
        auto decrypted_message = KS::CryptoHelper::ssr_decrypt(this->options_.public_filename.raw(), message);
        RETURN_VAL_IF_TRUE(decrypted_message.empty(), false);
        Glib::file_set_contents(this->options_.output_filename.raw(), decrypted_message);
        return true;
    }
    catch (const Glib::Error& e)
    {
        fmt::print(stderr, "{0}", e.what().c_str());
        return false;
    }
}

bool CmdParser::process_encrypt_file()
{
    if (this->options_.private_filename.empty())
    {
        fmt::print(stderr, "RSA private file isn't provided.");
        return false;
    }

    if (this->options_.output_filename.empty())
    {
        fmt::print(stderr, "Output file isn't provided.");
        return false;
    }

    try
    {
        auto message = Glib::file_get_contents(this->options_.encrypted_file);
        auto encrypted_message = KS::CryptoHelper::ssr_encrypt(this->options_.private_filename.raw(), message);
        // fmt::print("{0}  message: {1} encrypted_message: {2}", this->encrypt_in_filename_, message, encrypted_message);
        RETURN_VAL_IF_TRUE(encrypted_message.empty(), false);
        Glib::file_set_contents(this->options_.output_filename.raw(), encrypted_message);
        return true;
    }
    catch (const Glib::Error& e)
    {
        fmt::print(stderr, "{0}", e.what().c_str());
        return false;
    }
}

Glib::OptionEntry CmdParser::create_entry(const std::string& long_name,
                                          const Glib::ustring& description,
                                          int32_t flags)
{
    Glib::OptionEntry result;
    result.set_long_name(long_name);
    result.set_description(description.c_str());
    result.set_flags(flags);
    return result;
}

}  // namespace Crypto
}  // namespace KS
