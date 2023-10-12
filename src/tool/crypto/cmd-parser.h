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

#include <QCommandLineParser>
#include <QPair>
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
    // CommandOptions() : show_version(false) ,generate_rsa_key(false) {}
    // 打印版本号
    bool show_version;
    // 生成密钥对
    bool generate_rsa_key;
    // 解密文件
    QString decrypted_file;
    // 加密文件
    QString encrypted_file;
    // rsa公钥和私钥文件
    QString public_filename;
    QString private_filename;
    // 输出文件
    QString output_filename;
};

class CmdParser
{
public:
    CmdParser();
    virtual ~CmdParser(){};

    // 初始化
    void init();

    // 运行
    int run(QCoreApplication& a);

private:
    void initEntry();

    // 解密文件
    bool processDecryptFile();
    // 加密文件
    bool processEncryptFile();

    // Glib::OptionEntry create_entry(const std::string& long_name,
    //                                const QString& description,
    //                                int32_t flags = 0);

private:
    // Glib::OptionContext option_context_;
    // Glib::OptionGroup option_group_;

    CommandOptions options_;
    QCommandLineParser parser;
};
}  // namespace Crypto
}  // namespace KS