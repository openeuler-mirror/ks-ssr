/**
 * @file          /kiran-sse-manager/lib/base/crypto-helper.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/base64.h>
#include <cryptopp/des.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/rsa.h>

#include "lib/base/crypto-helper.h"
#include "lib/base/log.h"
#include "lib/base/str-utils.h"

using namespace CryptoPP;

ANONYMOUS_NAMESPACE_BEGIN
#if (CRYPTOPP_USE_AES_GENERATOR)
OFB_Mode<AES>::Encryption s_globalRNG;
#else
NonblockingRng s_globalRNG;
#endif
NAMESPACE_END

RandomNumberGenerator &GlobalRNG()
{
    return dynamic_cast<RandomNumberGenerator &>(s_globalRNG);
}

namespace Kiran
{
#define SSE_RSA_SEED_DEFAULT "kylinsec"
#define SSE_DES_KEY "kylinsec"

CryptoHelper::CryptoHelper()
{
}

CryptoHelper::~CryptoHelper()
{
}

std::string CryptoHelper::md5(const std::string &message)
{
    std::string result;
    Weak::MD5 md5;
    StringSource(message, true, new HashFilter(md5, new HexEncoder(new StringSink(result))));
    return result;
}

std::string CryptoHelper::md5_file(const std::string &filename)
{
    try
    {
        std::string result;
        Weak::MD5 md5;
        FileSource(filename.c_str(), true, new HashFilter(md5, new HexEncoder(new StringSink(result))));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        LOG_WARNING("%s.", e.what());
        return std::string();
    }
}

std::string CryptoHelper::base64_encrypt(const std::string &message)
{
    try
    {
        std::string result;
        StringSource(message, true, new Base64Encoder(new StringSink(result)));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        LOG_WARNING("%s.", e.what());
        return std::string();
    }
}

std::string CryptoHelper::base64_decrypt(const std::string &message)
{
    try
    {
        std::string result;
        StringSource(message, true, new Base64Decoder(new StringSink(result)));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        LOG_WARNING("%s.", e.what());
        return std::string();
    }
}

void CryptoHelper::generate_rsa_key(uint32_t key_length,
                                    const std::string &private_filename,
                                    const std::string &public_filename)
{
    try
    {
        RandomPool random_pool;
        random_pool.IncorporateEntropy((const byte *)SSE_RSA_SEED_DEFAULT, sizeof(SSE_RSA_SEED_DEFAULT));

        RSAES_OAEP_SHA_Decryptor priv(random_pool, key_length);
        HexEncoder priv_file(new Base64Encoder(new FileSink(private_filename.c_str())));
        priv.AccessMaterial().Save(priv_file);
        priv_file.MessageEnd();

        RSAES_OAEP_SHA_Encryptor pub(priv);
        HexEncoder pub_file(new Base64Encoder(new FileSink(public_filename.c_str())));
        pub.AccessMaterial().Save(pub_file);
        pub_file.MessageEnd();
    }
    catch (const CryptoPP::Exception &e)
    {
        LOG_WARNING("%s.", e.what());
    }
}

std::string CryptoHelper::rsa_encrypt(const std::string &public_filename,
                                      const std::string &message)
{
    try
    {
        FileSource pub_file(public_filename.c_str(), true, new Base64Decoder(new HexDecoder));
        RSAES_OAEP_SHA_Encryptor pub(pub_file);

        RandomPool random_pool;
        random_pool.IncorporateEntropy((const byte *)SSE_RSA_SEED_DEFAULT, sizeof(SSE_RSA_SEED_DEFAULT));

        if (message.size() > pub.FixedMaxPlaintextLength())
        {
            LOG_WARNING("The length(%d) of message is greater than the value(%d) which FixedMaxPlaintextLength return.",
                        message.size(),
                        pub.FixedMaxPlaintextLength());
            return std::string();
        }

        std::string result;
        StringSource(message, true, new PK_EncryptorFilter(random_pool, pub, new HexEncoder(new StringSink(result))));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        LOG_WARNING("%s.", e.what());
        return std::string();
    }
}

std::string CryptoHelper::rsa_decrypt(const std::string &private_filename, const std::string &ciphertext)
{
    try
    {
        FileSource priv_file(private_filename.c_str(), true, new Base64Decoder(new HexDecoder));
        RSAES_OAEP_SHA_Decryptor priv(priv_file);

        // 需要先HexDecoder后才能比较大小
        // if (ciphertext.size() > priv.FixedCiphertextLength())
        // {
        //     LOG_WARNING("The length(%d) of message is greater than the value(%d) which FixedCiphertextLength return.",
        //                 ciphertext.size(),
        //                 priv.FixedCiphertextLength());
        //     return std::string();
        // }

        std::string result;
        StringSource(ciphertext, true, new HexDecoder(new PK_DecryptorFilter(GlobalRNG(), priv, new StringSink(result))));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        LOG_WARNING("%s.", e.what());
        return std::string();
    }
}

bool CryptoHelper::rsa_sign_file(const std::string &private_filename,
                                 const std::string &message_filename,
                                 const std::string &signature_filename)
{
    try
    {
        FileSource priv_file(private_filename.c_str(), true, new Base64Decoder(new HexDecoder));

        RSASS<PKCS1v15, SHA1>::Signer priv(priv_file);
        FileSource(message_filename.c_str(), true, new SignerFilter(GlobalRNG(), priv, new HexEncoder(new FileSink(signature_filename.c_str()))));
        return true;
    }
    catch (const CryptoPP::Exception &e)
    {
        LOG_WARNING("%s.", e.what());
        return false;
    }
}

bool CryptoHelper::rsa_verify_file(const std::string &public_filename,
                                   const std::string &message_filename,
                                   const std::string &signature_filename)
{
    try
    {
        FileSource pub_file(public_filename.c_str(), true, new Base64Decoder(new HexDecoder));
        RSASS<PKCS1v15, SHA1>::Verifier pub(pub_file);

        FileSource signature_file(signature_filename.c_str(), true, new HexDecoder);
        RETURN_VAL_IF_FALSE(signature_file.MaxRetrievable() == pub.SignatureLength(), false);

        SecByteBlock signature(pub.SignatureLength());
        signature_file.Get(signature, signature.size());

        SignatureVerificationFilter *verifier_filter = new SignatureVerificationFilter(pub);
        verifier_filter->Put(signature, pub.SignatureLength());
        FileSource(message_filename.c_str(), true, verifier_filter);

        return verifier_filter->GetLastResult();
    }
    catch (const CryptoPP::Exception &e)
    {
        LOG_WARNING("%s.", e.what());
        return false;
    }
}

// static SecByteBlock get_des_key()
// {
//     SecByteBlock key(DES::DEFAULT_KEYLENGTH);
//     RandomPool random_pool;
//     random_pool.IncorporateEntropy((const byte *)SSE_RSA_SEED_DEFAULT, sizeof(SSE_RSA_SEED_DEFAULT));
//     random_pool.GenerateBlock(key, key.size());
//     return key;
// }

std::string CryptoHelper::des_encrypt(const std::string &message)
{
    try
    {
        std::string result;
        ECB_Mode<DES>::Encryption encoder;
        encoder.SetKey((const byte *)SSE_DES_KEY, sizeof(SSE_DES_KEY));
        // auto key = get_des_key();
        // encoder.SetKey(key, key.size());
        // fmt::print("key: {0}.", key.size());
        StringSource(message, true, new StreamTransformationFilter(encoder, new Base64Encoder(new StringSink(result))));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        LOG_WARNING("%s.", e.what());
        return std::string();
    }
}

std::string CryptoHelper::des_decrypt(const std::string &message)
{
    try
    {
        std::string result;
        ECB_Mode<DES>::Decryption decoder;
        decoder.SetKey((const byte *)SSE_DES_KEY, sizeof(SSE_DES_KEY));
        StringSource(message, true, new StreamTransformationFilter(decoder, new Base64Decoder(new StringSink(result))));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        LOG_WARNING("%s.", e.what());
        return std::string();
    }
}

std::string CryptoHelper::sse_encrypt(const std::string &public_filename, const std::string &message)
{
    LOG_DEBUG("key filename: %s. message: %s.", public_filename.c_str(), message.c_str());

    std::string result = CryptoHelper::base64_encrypt(message);
    RETURN_VAL_IF_FALSE(!result.empty(), std::string());

    auto message_md5 = CryptoHelper::md5(message);
    RETURN_VAL_IF_FALSE(!message_md5.empty(), std::string());

    LOG_DEBUG("message base64: %s, md5: %s.", result.c_str(), message_md5.c_str());

    result.push_back('@');
    auto md5_rsa = CryptoHelper::rsa_encrypt(public_filename, message_md5);
    RETURN_VAL_IF_FALSE(!md5_rsa.empty(), std::string());

    LOG_DEBUG("message md5 rsa: %s.", md5_rsa.c_str());

    result.append(md5_rsa);
    return result;
}

std::string CryptoHelper::sse_decrypt(const std::string &private_filename, const std::string &ciphertext)
{
    LOG_DEBUG("key filename: %s. ciphertext: %s.", private_filename.c_str(), ciphertext.c_str());

    auto fields = StrUtils::split_with_char(ciphertext, '@');
    RETURN_VAL_IF_FALSE(fields.size() == 2, std::string());

    // LOG_DEBUG("message base64: %s, md5 rsa: %s.", fields[0].c_str(), fields[1].c_str());

    auto plaintext = CryptoHelper::base64_decrypt(fields[0]);
    RETURN_VAL_IF_FALSE(!plaintext.empty(), std::string());

    auto plaintext_md5 = CryptoHelper::md5(plaintext);
    auto rsa_md5 = CryptoHelper::rsa_decrypt(private_filename, fields[1]);

    LOG_DEBUG("plaintext md5: %s, rsa md5: %s.", plaintext_md5.c_str(), rsa_md5.c_str());

    if (plaintext_md5 == rsa_md5)
    {
        return plaintext;
    }
    else
    {
        LOG_WARNING("The ciphertext is invalid.");
        return std::string();
    }
}

}  // namespace Kiran