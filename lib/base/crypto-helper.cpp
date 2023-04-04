/**
 * Copyright (c) 2022 ~ 2023 KylinSec Co., Ltd. 
 * ks-sc is licensed under Mulan PSL v2.
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

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/des.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/rsa.h>
#include <qt5-log-i.h>

#include "include/sc-marcos.h"
#include "lib/base/crypto-helper.h"

using namespace CryptoPP;

ANONYMOUS_NAMESPACE_BEGIN
#if (CRYPTOPP_USE_AES_GENERATOR)
OFB_Mode<AES>::Encryption s_globalRNG;
#else
NonblockingRng s_globalRNG;
#endif
NAMESPACE_END

RandomNumberGenerator &global_rng()
{
    return dynamic_cast<RandomNumberGenerator &>(s_globalRNG);
}

namespace KS
{
CryptoHelper::CryptoHelper()
{
}

CryptoHelper::~CryptoHelper()
{
}

void CryptoHelper::generateRsaKey(uint32_t key_length,
                                    QString &privateKey,
                                    QString &publicKey)
{
    auto privateKeyStd = privateKey.toStdString();
    auto publicKeyStd = publicKey.toStdString();
        RSAES_OAEP_SHA_Decryptor rsa_decryptor(global_rng(), key_length);
        HexEncoder private_sink(new Base64Encoder(new StringSink(privateKeyStd)));
        rsa_decryptor.AccessMaterial().Save(private_sink);
        private_sink.MessageEnd();

        RSAES_OAEP_SHA_Encryptor rsa_encryptor(rsa_decryptor);
        HexEncoder public_sink(new Base64Encoder(new StringSink(publicKeyStd)));
        rsa_encryptor.AccessMaterial().Save(public_sink);
        public_sink.MessageEnd();
        privateKey = QString::fromStdString(privateKeyStd);
        publicKey = QString::fromStdString(publicKeyStd);
}

QString CryptoHelper::rsaEncrypt(const QString &publicKey,
                                      const QString &message)
{
    RETURN_VAL_IF_TRUE(message.isEmpty(), QString());

    try
    {
        RandomPool random_pool;
        StringSource public_source(publicKey.toStdString(), true, new Base64Decoder(new HexDecoder));
        RSAES_OAEP_SHA_Encryptor rsa_encryptor(public_source);

        if (message.size() > rsa_encryptor.FixedMaxPlaintextLength())
        {
            KLOG_WARNING("The length(%d) of message is greater than the value(%d) which FixedMaxPlaintextLength return.",
                         message.size(),
                         rsa_encryptor.FixedMaxPlaintextLength());
            return QString();
        }

        std::string result;
        StringSource(message.toStdString(), true, new PK_EncryptorFilter(random_pool, rsa_encryptor, new HexEncoder(new StringSink(result))));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

QString CryptoHelper::rsaDecrypt(const QString &privateKey, const QString &ciphertext)
{
    RETURN_VAL_IF_TRUE(ciphertext.isEmpty(), QString());

    try
    {
        RandomPool random_pool;
        StringSource private_source(privateKey.toStdString(), true, new Base64Decoder(new HexDecoder));
        RSAES_OAEP_SHA_Decryptor priv(private_source);

        std::string result;
        StringSource(ciphertext.toStdString(), true, new HexDecoder(new PK_DecryptorFilter(random_pool, priv, new StringSink(result))));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

QString CryptoHelper::aesEncrypt(const QString &message, const QString &key)
{
    try
    {
        std::string result;
        ECB_Mode<AES>::Encryption encoder;

        encoder.SetKey((const byte *)key.toStdString().c_str(), key.length());
        StringSource(message.toStdString(), true, new StreamTransformationFilter(encoder, new Base64Encoder(new StringSink(result))));

        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

QString CryptoHelper::aesDecrypt(const QString &message, const QString &key)
{
    try
    {
        std::string result;
        ECB_Mode<AES>::Decryption decoder;

        decoder.SetKey((const byte *)key.toStdString().c_str(), key.length());
        StringSource(message.toStdString(), true, new Base64Decoder(new StreamTransformationFilter(decoder, new StringSink(result))));

        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

}  // namespace KS
