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
// #include <glib/gi18n.h>
#include <QFile>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

namespace KS
{
namespace Crypto
{
#define BR_RSA_LENGTH 1024

CmdParser::CmdParser() /** : option_group_(PROJECT_NAME, "group options") **/
{
}

void CmdParser::init()
{
    this->parser.addHelpOption();
    this->parser.addVersionOption();
    this->parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    this->parser.addOption({"generate-rsa-key",
                            _("Generate public and private keys for RSA."), "path"});
    this->parser.addOption({"decrypt-file",
                            _("Decrypt a file."), "path"});
    this->parser.addOption({"encrypt-file",
                            _("Encrypt a file."), "path"});
    this->parser.addOption({"public-key",
                            _("RSA public file path."), "path"});
    this->parser.addOption({"private-key",
                            _("RSA private file path."), "path"});
    this->parser.addOption({"output-file",
                            _("Output file path."), "path"});
    // this->option_group_.add_entry(MiscUtils::create_option_entry("version", N_("Output version infomation and exit.")),
    //                               this->options_.show_version);

    // this->option_group_.add_entry(MiscUtils::create_option_entry("generate-rsa-key", N_("Generate public and private keys for RSA.")),
    //                               this->options_.generate_rsa_key);

    // this->option_group_.add_entry(MiscUtils::create_option_entry("decrypt-file", N_("Decrypt a file.")),
    //                               this->options_.decrypted_file);

    // this->option_group_.add_entry(MiscUtils::create_option_entry("encrypt-file", N_("Encrypt a file.")),
    //                               this->options_.encrypted_file);

    // this->option_group_.add_entry(MiscUtils::create_option_entry("public-key", N_("RSA public file path.")),
    //                               this->options_.public_filename);

    // this->option_group_.add_entry(MiscUtils::create_option_entry("private-key", N_("RSA private file path.")),
    //                               this->options_.private_filename);

    // this->option_group_.add_entry(MiscUtils::create_option_entry("output-file", N_("Output file path.")),
    //                               this->options_.output_filename);

    // this->option_group_.set_translation_domain(PROJECT_NAME);
    // this->option_context_.set_main_group(this->option_group_);
}

int CmdParser::run(QCoreApplication& a)
{
    this->parser.process(a);

    this->options_ = {
        QVariant(this->parser.value("version")).toBool(),
        QVariant(this->parser.value("generate-rsa-key")).toBool(),
        this->parser.value("decrypt-file"),
        this->parser.value("encrypt-file"),
        this->parser.value("public-key"),
        this->parser.value("private-key"),
        this->parser.value("output-file"),
    };

    if (this->options_.generate_rsa_key)
    {
        KS::CryptoHelper::generate_rsa_key(BR_RSA_LENGTH, "br-public.key", "br-private.key");
    }
    else if (!this->options_.decrypted_file.isEmpty())
    {
        RETURN_VAL_IF_FALSE(this->processDecryptFile(), EXIT_FAILURE);
    }
    else if (!this->options_.encrypted_file.isEmpty())
    {
        RETURN_VAL_IF_FALSE(this->processEncryptFile(), EXIT_FAILURE);
    }
    else
    {
        // 默认显示版本
        cout << QString("%1: %1\n").arg(a.applicationName(), a.applicationVersion()).toStdString() << endl;
    }
    return EXIT_SUCCESS;
}

bool CmdParser::processDecryptFile()
{
    if (this->options_.public_filename.isEmpty())
    {
        cerr << "RSA public file isn't provided." << endl;
        // fmt::print(stderr, "RSA public file isn't provided.");
        return false;
    }

    if (this->options_.output_filename.isEmpty())
    {
        cerr << "Output file isn't provided." << endl;
        // fmt::print(stderr, "Output file isn't provided.");
        return false;
    }

    QFile decrypted_file(this->options_.decrypted_file);
    QFile output_file(this->options_.output_filename);
    if (!decrypted_file.open(QIODevice::OpenModeFlag::ReadOnly) ||
        !output_file.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Truncate))
    {
        cerr << "Failed to open decrypted file or output file " << endl;
        cerr << "decrypted file : " << this->options_.decrypted_file.toStdString() << endl;
        cerr << "output file : " << this->options_.output_filename.toStdString() << endl;
        return false;
    }
    auto decrypted_message = KS::CryptoHelper::brDecrypt(this->options_.public_filename, decrypted_file.readAll());
    RETURN_VAL_IF_TRUE(decrypted_message.isEmpty(), false);
    output_file.write(decrypted_message.toUtf8());
    decrypted_file.close();
    output_file.close();
    return true;
}

bool CmdParser::processEncryptFile()
{
    if (this->options_.private_filename.isEmpty())
    {
        cerr << "RSA private file isn't provided." << endl;
        return false;
    }

    if (this->options_.output_filename.isEmpty())
    {
        cerr << "Output file isn't provided." << endl;
        return false;
    }

    QFile encrypted_file(this->options_.encrypted_file);
    QFile output_file(this->options_.output_filename);
    if (!encrypted_file.open(QIODevice::OpenModeFlag::ReadOnly) ||
        !output_file.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Truncate))
    {
        cerr << "Failed to open encrypted file or output file " << endl;
        cerr << "encrypted file : " << this->options_.encrypted_file.toStdString() << endl;
        cerr << "output file : " << this->options_.output_filename.toStdString() << endl;
        return false;
    }
    auto encrypted_message = KS::CryptoHelper::brEncrypt(this->options_.private_filename, encrypted_file.readAll());
    // fmt::print("{0}  message: {1} encrypted_message: {2}", this->encrypt_in_filename_, message, encrypted_message);
    RETURN_VAL_IF_TRUE(encrypted_message.isEmpty(), false);
    output_file.write(encrypted_message.toUtf8());
    // Glib::file_set_contents(this->options_.output_filename.raw(), encrypted_message);
    encrypted_file.close();
    output_file.close();
    return true;
}

}  // namespace Crypto
}  // namespace KS
