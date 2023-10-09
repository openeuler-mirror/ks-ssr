/**
 * Copyright (c) 2022 ~ 2023 KylinSec Co., Ltd. 
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QString>
#include <cstdint>
#include <string>

namespace KS
{
#define DEFAULT_DES_KEY "scdaemon"
#define DEFAULT_AES_KEY "kiranscdaemonaes"

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
    static QString rsaEncrypt(const QString &publicKey, const QString &message);
    static QString rsaDecrypt(const QString &privateKey, const QString &ciphertext);

    // aes加密和解密
    static QString aesEncrypt(const QString &message, const QString &key = DEFAULT_AES_KEY);
    static QString aesDecrypt(const QString &message, const QString &key = DEFAULT_AES_KEY);
};
}  // namespace KS
