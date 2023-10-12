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

#include <gtest/gtest.h>
#include "br-config.h"
#include "lib/base/base.h"

TEST(RSATest, EncryptAndDecrypt)
{
    std::string raw_text("Hello world!");
    auto private_filename = Glib::build_filename(std::vector<std::string>{PROJECT_SOURCE_DIR, "data", "br-private.key"});
    auto public_filename = Glib::build_filename(std::vector<std::string>{PROJECT_SOURCE_DIR, "data", "br-public.key"});

    auto encrypted_text = KS::CryptoHelper::rsaEncryptString(private_filename, raw_text);
    auto decrypted_text = KS::CryptoHelper::rsaDecryptString(public_filename, encrypted_text);

    ASSERT_STREQ(raw_text.c_str(), decrypted_text.c_str());
}

TEST(DESTest, EncryptAndDecrypt)
{
    KS::CryptoHelper crypto_helper;
    std::string raw_text("aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbccccccccccccccccccccccccddddddddddddddd");

    auto encrypted_text = KS::CryptoHelper::desEncrypt(raw_text);
    auto decrypted_text = KS::CryptoHelper::desDecrypt(encrypted_text);
    // fmt::print("xxxxxxxxxxx {0} {1}.", encrypted_text.c_str(), encrypted_text.size());
    ASSERT_STREQ(raw_text.c_str(), decrypted_text.c_str());
}