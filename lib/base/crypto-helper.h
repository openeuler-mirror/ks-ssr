/**
 * @file          /kiran-ssr-manager/lib/base/crypto-helper.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <cstdint>
#include <string>

namespace Kiran
{
class CryptoHelper
{
public:
    CryptoHelper();
    virtual ~CryptoHelper();

    // md5
    static std::string md5(const std::string &message);
    static std::string md5_file(const std::string &filename);

    // base64加密和解密
    static std::string base64_encrypt(const std::string &message);
    static std::string base64_decrypt(const std::string &message);

    // 生成rsa公钥和私钥对
    static void generate_rsa_key(uint32_t key_length,
                                 const std::string &private_filename,
                                 const std::string &public_filename);

    // rsa加密和解密
    static std::string rsa_encrypt(const std::string &public_filename, const std::string &message);
    static std::string rsa_decrypt(const std::string &private_filename, const std::string &ciphertext);

    // rsa签名
    static bool rsa_sign_file(const std::string &private_filename,
                              const std::string &message_filename,
                              const std::string &signature_filename);
    static bool rsa_verify_file(const std::string &public_filename,
                                const std::string &message_filename,
                                const std::string &signature_filename);

    // des加密和解密
    static std::string des_encrypt(const std::string &message);
    static std::string des_decrypt(const std::string &message);

    // 安全加固的加密和解密算法
    static std::string ssr_encrypt(const std::string &public_filename, const std::string &message);
    static std::string ssr_decrypt(const std::string &private_filename, const std::string &ciphertext);
};
}  // namespace Kiran