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

#pragma once

#include <QString>
#include <cstdint>
#include <string>

namespace KS
{
#define DEFAULT_DES_KEY "scdaemon"
#define DEFAULT_AES_KEY "kiranscdaemonaes"
#define BR_DES_KEY "kylinsec"

class CryptoHelper
{
public:
    CryptoHelper();
    virtual ~CryptoHelper();

    // 生成rsa公钥和私钥对
    static void generateRsaKey(uint32_t keyLength,
                               QString &privateKey,
                               QString &publicKey);

    // rsa加密和解密
    static QString rsaEncryptString(const QString &publicKey, const QString &message);
    static QString rsaDecryptString(const QString &privateKey, const QString &ciphertext);
    static QString rsaEncryptFile(const QString &public_filename, const QString &message);
    static QString rsaDecryptFile(const QString &private_filename, const QString &ciphertext);

    // aes加密和解密
    static QString aesEncrypt(const QString &message, const QString &key = DEFAULT_AES_KEY);
    static QString aesDecrypt(const QString &message, const QString &key = DEFAULT_AES_KEY);
    // md5
    static QString md5(const QString &message);
    static QString md5File(const QString &filename);

    // base64加密和解密
    static QString base64Encrypt(const QString &message);
    static QString base64Decrypt(const QString &message);

    // 生成rsa公钥和私钥对
    static void generate_rsa_key(uint32_t key_length,
                                 const QString &private_filename,
                                 const QString &public_filename);

    // rsa签名
    static bool rsaSignFile(const QString &private_filename,
                              const QString &message_filename,
                              const QString &signature_filename);
    static bool rsaVerifyFile(const QString &public_filename,
                                const QString &message_filename,
                                const QString &signature_filename);

    // des加密和解密
    static QString desEncrypt(const QString &message, const QString &key = BR_DES_KEY);
    static QString desDecrypt(const QString &message, const QString &key = BR_DES_KEY);

    // 安全加固的加密和解密算法
    static QString brEncrypt(const QString &public_filename, const QString &message);
    static QString brDecrypt(const QString &private_filename, const QString &ciphertext);
};
}  // namespace KS
