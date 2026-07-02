/**
 * @file          /kiran-ssr-manager/src/tool/ssr-cmd-parser.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/base.h"

namespace Kiran
{
enum OperationType
{
    OPERATION_TYPE_NONE = 0,
    // 打印版本号
    OPERATION_TYPE_PRINT_VERSION,
    // 生成密钥对
    OPERATION_TYPE_GENERATE_RSA_KEY,
    // 解密文件
    OPERATION_TYPE_DECRYPT_FILE,
    // 加密文件
    OPERATION_TYPE_ENCRYPT_FILE,

};

class SSRCmdParser
{
public:
    SSRCmdParser();
    virtual ~SSRCmdParser(){};

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

    // 执行的操作
    OperationType operation_type_;
    // rsa公钥和私钥文件
    std::string public_filename_;
    std::string private_filename_;
    // 加密的文件
    std::string decrypt_in_filename_;
    // 需要加密的文件
    std::string encrypt_in_filename_;
    // 输出文件
    std::string output_filename_;
};
}  // namespace Kiran