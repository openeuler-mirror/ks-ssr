/**
 * @file          /kiran-ssr-manager/src/tool/crypto/cmd-parser.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/tool/crypto/cmd-parser.h"
#include <glib/gi18n.h>

namespace Kiran
{
namespace Crypto
{
#define SSR_RSA_LENGTH 1024

CmdParser::CmdParser() : option_group_(PROJECT_NAME, "group options"),
                         operation_type_(OperationType::OPERATION_TYPE_NONE)
{
}

void CmdParser::init()
{
    Glib::OptionEntry entry;

    entry = this->create_entry("version", N_("Output version infomation and exit."), Glib::OptionEntry::FLAG_NO_ARG);
    this->option_group_.add_entry(entry, [this](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        this->operation_type_ = OperationType::OPERATION_TYPE_PRINT_VERSION;
        return true;
    });

    entry = this->create_entry("generate-rsa-key", N_("Generate public and private keys for RSA."), Glib::OptionEntry::FLAG_NO_ARG);
    this->option_group_.add_entry(entry, [this](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        this->operation_type_ = OperationType::OPERATION_TYPE_GENERATE_RSA_KEY;
        return true;
    });

    entry = this->create_entry("decrypt-file", N_("Decrypt a file."));
    this->option_group_.add_entry(entry, [this](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        this->operation_type_ = OperationType::OPERATION_TYPE_DECRYPT_FILE;
        this->decrypt_in_filename_ = value;
        return true;
    });

    entry = this->create_entry("encrypt-file", N_("Encrypt a file."));
    this->option_group_.add_entry(entry, [this](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        this->operation_type_ = OperationType::OPERATION_TYPE_ENCRYPT_FILE;
        this->encrypt_in_filename_ = value;
        return true;
    });

    entry = this->create_entry("public-key", N_("RSA public file path."));
    this->option_group_.add_entry_filename(entry, this->public_filename_);

    entry = this->create_entry("private-key", N_("RSA private file path."));
    this->option_group_.add_entry_filename(entry, this->private_filename_);

    entry = this->create_entry("output-file", N_("Output file path."));
    this->option_group_.add_entry_filename(entry, this->output_filename_);

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

    switch (this->operation_type_)
    {
    case OperationType::OPERATION_TYPE_PRINT_VERSION:
    {
        auto program_name = Glib::path_get_basename(argv[0]);
        fmt::print("%s: %s\n", program_name.c_str(), PROJECT_VERSION);
        break;
    }
    case OperationType::OPERATION_TYPE_GENERATE_RSA_KEY:
        // 这里对私钥和公钥进行了互换，因为需要使用私钥进行加密，公钥进行解密。
        Kiran::CryptoHelper::generate_rsa_key(SSR_RSA_LENGTH,
                                              "ssr-public.key",
                                              "ssr-private.key");
        break;
    case OperationType::OPERATION_TYPE_DECRYPT_FILE:
        RETURN_VAL_IF_FALSE(this->process_decrypt_file(), EXIT_FAILURE);
        break;
    case OperationType::OPERATION_TYPE_ENCRYPT_FILE:
        RETURN_VAL_IF_FALSE(this->process_encrypt_file(), EXIT_FAILURE);
        break;
    default:
        break;
    }
    return EXIT_SUCCESS;
}

bool CmdParser::process_decrypt_file()
{
    if (this->public_filename_.empty())
    {
        fmt::print(stderr, "RSA public file isn't provided.");
        return false;
    }

    if (this->output_filename_.empty())
    {
        fmt::print(stderr, "Output file isn't provided.");
        return false;
    }

    try
    {
        auto message = Glib::file_get_contents(this->decrypt_in_filename_);
        auto decrypted_message = Kiran::CryptoHelper::ssr_decrypt(this->public_filename_, message);
        RETURN_VAL_IF_TRUE(decrypted_message.empty(), false);
        Glib::file_set_contents(this->output_filename_, decrypted_message);
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
    if (this->private_filename_.empty())
    {
        fmt::print(stderr, "RSA private file isn't provided.");
        return false;
    }

    if (this->output_filename_.empty())
    {
        fmt::print(stderr, "Output file isn't provided.");
        return false;
    }

    try
    {
        auto message = Glib::file_get_contents(this->encrypt_in_filename_);
        auto encrypted_message = Kiran::CryptoHelper::ssr_encrypt(this->private_filename_, message);
        // fmt::print("{0}  message: {1} encrypted_message: {2}", this->encrypt_in_filename_, message, encrypted_message);
        RETURN_VAL_IF_TRUE(encrypted_message.empty(), false);
        Glib::file_set_contents(this->output_filename_, encrypted_message);
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
}  // namespace Kiran
