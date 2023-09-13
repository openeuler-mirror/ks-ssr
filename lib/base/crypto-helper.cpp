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

#pragma message("此文件需要调整，有较为严重的代码冗余")

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

#include "crypto-helper.h"
#include "include/ssr-marcos.h"
#include "lib/base/base.h"
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

RandomNumberGenerator &GlobalRNG()
{
    return dynamic_cast<RandomNumberGenerator &>(s_globalRNG);
}
namespace KS
{
#define BR_RSA_SEED_DEFAULT "kylinsec"

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
    RSAES_OAEP_SHA_Decryptor rsaDecryptor(global_rng(), key_length);
    HexEncoder private_sink(new Base64Encoder(new StringSink(privateKeyStd)));
    rsaDecryptor.AccessMaterial().Save(private_sink);
    private_sink.MessageEnd();

    RSAES_OAEP_SHA_Encryptor rsaEncryptor(rsaDecryptor);
    HexEncoder public_sink(new Base64Encoder(new StringSink(publicKeyStd)));
    rsaEncryptor.AccessMaterial().Save(public_sink);
    public_sink.MessageEnd();
    privateKey = QString::fromStdString(privateKeyStd);
    publicKey = QString::fromStdString(publicKeyStd);
}

QString CryptoHelper::rsaEncryptString(const QString &publicKey,
                                       const QString &message)
{
    RETURN_VAL_IF_TRUE(message.isEmpty(), QString());

    try
    {
        RandomPool random_pool;
        StringSource public_source(publicKey.toStdString(), true, new Base64Decoder(new HexDecoder));
        RSAES_OAEP_SHA_Encryptor rsaEncryptor(public_source);

        if (message.size() > rsaEncryptor.FixedMaxPlaintextLength())
        {
            KLOG_WARNING("The length(%d) of message is greater than the value(%d) which FixedMaxPlaintextLength return.",
                         message.size(),
                         rsaEncryptor.FixedMaxPlaintextLength());
            return QString();
        }

        std::string result;
        StringSource(message.toStdString(), true, new PK_EncryptorFilter(random_pool, rsaEncryptor, new HexEncoder(new StringSink(result))));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

QString CryptoHelper::rsaDecryptString(const QString &privateKey, const QString &ciphertext)
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

QString CryptoHelper::rsaEncryptFile(const QString &public_filename, const QString &message)
{
    try
    {
        FileSource pub_file(public_filename.toLatin1().data(), true, new Base64Decoder(new HexDecoder));
        RSAES_OAEP_SHA_Encryptor pub(pub_file);

        RandomPool random_pool;
        random_pool.IncorporateEntropy((const byte *)BR_RSA_SEED_DEFAULT, sizeof(BR_RSA_SEED_DEFAULT));

        if (message.size() > pub.FixedMaxPlaintextLength())
        {
            KLOG_WARNING("The length(%d) of message is greater than the value(%d) which FixedMaxPlaintextLength return.",
                         message.size(),
                         pub.FixedMaxPlaintextLength());
            return QString();
        }

        // StringSink 偏特化了 std::string ,所以这里使用 std::string
        std::string result;
        StringSource(message.toLatin1().data(), true,
                     new PK_EncryptorFilter(random_pool, pub, new HexEncoder(new StringSink(result))));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

QString CryptoHelper::rsaDecryptFile(const QString &private_filename, const QString &ciphertext)
{
    try
    {
        FileSource priv_file(private_filename.toLatin1().data(), true, new Base64Decoder(new HexDecoder));
        RSAES_OAEP_SHA_Decryptor priv(priv_file);

        // 需要先HexDecoder后才能比较大小
        // if (ciphertext.size() > priv.FixedCiphertextLength())
        // {
        //     KLOG_WARNING("The length(%d) of message is greater than the value(%d) which FixedCiphertextLength return.",
        //                 ciphertext.size(),
        //                 priv.FixedCiphertextLength());
        //     return QString();
        // }

        // StringSink 偏特化了 std::string ,所以这里使用 std::string
        std::string result;
        StringSource(ciphertext.toLatin1().data(), true,
                     new HexDecoder(new PK_DecryptorFilter(GlobalRNG(), priv, new StringSink(result))));
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

QString CryptoHelper::md5(const QString &message)
{
    // StringSink 偏特化了 std::string ,所以这里使用 std::string
    std::string result;
    Weak::MD5 md5;
    StringSource(message.toLatin1().data(), true, new HashFilter(md5, new HexEncoder(new StringSink(result))));
    return QString::fromStdString(result);
}

QString CryptoHelper::md5File(const QString &filename)
{
    try
    {
        // StringSink 偏特化了 std::string ,所以这里使用 std::string
        std::string result;
        Weak::MD5 md5;
        FileSource(filename.toLatin1().data(), true, new HashFilter(md5, new HexEncoder(new StringSink(result))));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

QString CryptoHelper::base64Encrypt(const QString &message)
{
    try
    {
        // StringSink 偏特化了 std::string ,所以这里使用 std::string
        std::string result;
        StringSource(message.toLatin1().data(), true, new Base64Encoder(new StringSink(result)));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

QString CryptoHelper::base64Decrypt(const QString &message)
{
    try
    {
        // StringSink 偏特化了 std::string ,所以这里使用 std::string
        std::string result;
        StringSource(message.toLatin1().data(), true, new Base64Decoder(new StringSink(result)));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

void CryptoHelper::generate_rsa_key(uint32_t key_length,
                                    const QString &private_filename,
                                    const QString &public_filename)
{
    try
    {
        RandomPool random_pool;
        random_pool.IncorporateEntropy((const byte *)BR_RSA_SEED_DEFAULT, sizeof(BR_RSA_SEED_DEFAULT));

        RSAES_OAEP_SHA_Decryptor priv(random_pool, key_length);
        HexEncoder priv_file(new Base64Encoder(new FileSink(private_filename.toLatin1().data())));
        priv.AccessMaterial().Save(priv_file);
        priv_file.MessageEnd();

        RSAES_OAEP_SHA_Encryptor pub(priv);
        HexEncoder pub_file(new Base64Encoder(new FileSink(public_filename.toLatin1().data())));
        pub.AccessMaterial().Save(pub_file);
        pub_file.MessageEnd();
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
    }
}

bool CryptoHelper::rsaSignFile(const QString &private_filename,
                               const QString &message_filename,
                               const QString &signature_filename)
{
    try
    {
        FileSource priv_file(private_filename.toLatin1().data(), true, new Base64Decoder(new HexDecoder));

        RSASS<PKCS1v15, SHA1>::Signer priv(priv_file);
        FileSource(message_filename.toLatin1().data(), true, new SignerFilter(GlobalRNG(), priv, new HexEncoder(new FileSink(signature_filename.toLatin1().data()))));
        return true;
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return false;
    }
}

bool CryptoHelper::rsaVerifyFile(const QString &public_filename,
                                 const QString &message_filename,
                                 const QString &signature_filename)
{
    try
    {
        FileSource pub_file(public_filename.toLatin1().data(), true, new Base64Decoder(new HexDecoder));
        RSASS<PKCS1v15, SHA1>::Verifier pub(pub_file);

        FileSource signature_file(signature_filename.toLatin1().data(), true, new HexDecoder);
        RETURN_VAL_IF_FALSE(signature_file.MaxRetrievable() == pub.SignatureLength(), false);

        SecByteBlock signature(pub.SignatureLength());
        signature_file.Get(signature, signature.size());

        SignatureVerificationFilter *verifier_filter = new SignatureVerificationFilter(pub);
        verifier_filter->Put(signature, pub.SignatureLength());
        FileSource(message_filename.toLatin1().data(), true, verifier_filter);

        return verifier_filter->GetLastResult();
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return false;
    }
}

// static SecByteBlock get_des_key()
// {
//     SecByteBlock key(DES::DEFAULT_KEYLENGTH);
//     RandomPool random_pool;
//     random_pool.IncorporateEntropy((const byte *)BR_RSA_SEED_DEFAULT, sizeof(BR_RSA_SEED_DEFAULT));
//     random_pool.GenerateBlock(key, key.size());
//     return key;
// }

QString CryptoHelper::desEncrypt(const QString &message, const QString &key)
{
    try
    {
        // StringSink 偏特化了 std::string ,所以这里使用 std::string
        std::string result;
        ECB_Mode<DES>::Encryption encoder;
        // 这里的key长度必须为8
        encoder.SetKey((const byte *)key.toLatin1().data(), key.length());
        // auto key = get_des_key();
        // encoder.SetKey(key, key.size());
        StringSource(message.toLatin1().data(), true, new StreamTransformationFilter(encoder, new Base64Encoder(new StringSink(result))));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

QString CryptoHelper::desDecrypt(const QString &message, const QString &key)
{
    try
    {
        // StringSink 偏特化了 std::string ,所以这里使用 std::string
        std::string result;
        ECB_Mode<DES>::Decryption decoder;
        decoder.SetKey((const byte *)key.toLatin1().data(), key.length());
        StringSource(message.toLatin1().data(), true,
                     new Base64Decoder(new StreamTransformationFilter(decoder, new StringSink(result))));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

QString CryptoHelper::brEncrypt(const QString &public_filename, const QString &message)
{
    KLOG_DEBUG("key filename: %s. message: %s.", public_filename.toLatin1().data(), message.toLatin1().data());

    QString result = CryptoHelper::base64Encrypt(message);
    RETURN_VAL_IF_FALSE(!result.isEmpty(), QString());

    auto message_md5 = CryptoHelper::md5(message);
    RETURN_VAL_IF_FALSE(!message_md5.isEmpty(), QString());

    KLOG_DEBUG("message base64: %s, md5: %s.", result.toLatin1().data(), message_md5.toLatin1().data());

    result.push_back('@');
    auto md5_rsa = CryptoHelper::rsaEncryptFile(public_filename, message_md5);
    RETURN_VAL_IF_FALSE(!md5_rsa.isEmpty(), QString());

    KLOG_DEBUG("message md5 rsa: %s.", md5_rsa.toLatin1().data());

    result.append(md5_rsa);
    return result;
}

QString CryptoHelper::brDecrypt(const QString &private_filename, const QString &ciphertext)
{
    KLOG_DEBUG("key filename: %s. ciphertext: %s.", private_filename.toLatin1().data(), ciphertext.toLatin1().data());

    auto fields = StrUtils::splitWithChar(ciphertext, '@');
    RETURN_VAL_IF_FALSE(fields.size() == 2, QString());

    // KLOG_DEBUG("message base64: %s, md5 rsa: %s.", fields[0].toLatin1().data(), fields[1].toLatin1().data());

    auto plaintext = CryptoHelper::base64Decrypt(fields[0]);
    RETURN_VAL_IF_FALSE(!plaintext.isEmpty(), QString());

    auto plaintext_md5 = CryptoHelper::md5(plaintext);
    auto rsa_md5 = CryptoHelper::rsaDecryptFile(private_filename, fields[1]);

    KLOG_DEBUG("plaintext md5: %s, rsa md5: %s.", plaintext_md5.toLatin1().data(), rsa_md5.toLatin1().data());

    if (plaintext_md5 == rsa_md5)
    {
        return plaintext;
    }
    else
    {
        KLOG_WARNING("The ciphertext is invalid.");
        return QString();
    }
}

}  // namespace KS
