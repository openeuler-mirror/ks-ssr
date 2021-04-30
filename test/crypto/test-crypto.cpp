/**
 * @file          /kiran-sse-manager/test/crypto/test-crypto.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <gtest/gtest.h>
#include "lib/base/base.h"
#include "sse-config.h"

TEST(RSATest, EncryptAndDecrypt)
{
    std::string raw_text("Hello world!");
    auto private_filename = Glib::build_filename(std::vector<std::string>{PROJECT_SOURCE_DIR, "data", "sse-private.key"});
    auto public_filename = Glib::build_filename(std::vector<std::string>{PROJECT_SOURCE_DIR, "data", "sse-public.key"});

    auto encrypted_text = Kiran::CryptoHelper::rsa_encrypt(private_filename, raw_text);
    auto decrypted_text = Kiran::CryptoHelper::rsa_decrypt(public_filename, encrypted_text);

    ASSERT_STREQ(raw_text.c_str(), decrypted_text.c_str());
}

TEST(DESTest, EncryptAndDecrypt)
{
    Kiran::CryptoHelper crypto_helper;
    std::string raw_text("aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbccccccccccccccccccccccccddddddddddddddd");

    auto encrypted_text = Kiran::CryptoHelper::des_encrypt(raw_text);
    auto decrypted_text = Kiran::CryptoHelper::des_decrypt(encrypted_text);
    // fmt::print("{0}.", encrypted_text.c_str());
    ASSERT_STREQ(raw_text.c_str(), decrypted_text.c_str());
}