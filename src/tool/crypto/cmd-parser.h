/**
 * @file          /ks-ssr-manager/src/tool/crypto/cmd-parser.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/base.h"

namespace KS
{
namespace Crypto
{
enum OperationType
{
    OPERATION_TYPE_NONE = 0,
    OPERATION_TYPE_PRINT_VERSION,
    OPERATION_TYPE_GENERATE_RSA_KEY,
    OPERATION_TYPE_DECRYPT_FILE,
    OPERATION_TYPE_ENCRYPT_FILE,

};

struct CommandOptions
{
    CommandOptions() : show_version(false) {}
    // 打印版本号
    bool show_version = false;
    // 生成密钥对
    bool generate_rsa_key = false;
    // 解密文件
    Glib::ustring decrypted_file;
    // 加密文件
    Glib::ustring encrypted_file;
    // rsa公钥和私钥文件
    Glib::ustring public_filename;
    Glib::ustring private_filename;
    // 输出文件
    Glib::ustring output_filename;
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
    void init_entry();

    // 解密文件
    bool process_decrypt_file();
    // 加密文件
    bool process_encrypt_file();

    Glib::OptionEntry create_entry(const std::string& long_name,
                                   const Glib::ustring& description,
                                   int32_t flags = 0);

private:
    Glib::OptionContext option_context_;
    Glib::OptionGroup option_group_;

    CommandOptions options_;
};
}  // namespace Crypto
}  // namespace KS