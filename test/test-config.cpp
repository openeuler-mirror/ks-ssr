/**
 * @file          /kiran-ssr-manager/test/test-config.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <gtest/gtest.h>
#include "lib/base/base.h"
#include "lib/config/config-plain.h"

using namespace Kiran;

const std::string login_defs = R""(#
# Please note that the parameters in this configuration file control the
# behavior of the tools from the shadow-utils component. None of these
# tools uses the PAM mechanism, and the utilities that use PAM (such as the
# passwd command) should therefore be configured elsewhere. Refer to
# /etc/pam.d/system-auth for more information.
#

# Password aging controls:
#
#	PASS_MAX_DAYS	Maximum number of days a password may be used.
#	PASS_MIN_DAYS	Minimum number of days allowed between password changes.
#	PASS_MIN_LEN	Minimum acceptable password length.
#	PASS_WARN_AGE	Number of days warning given before a password expires.
#
PASS_MAX_DAYS	99999
TEST_BOOL_STR   false
TEST_BOOL_NUM   1


#
# Min/max values for automatic uid selection in useradd
#
UID_MIN                  1000
UID_MAX                 60000
# System accounts
SYS_UID_MIN               201
SYS_UID_MAX               999

#
# Min/max values for automatic gid selection in groupadd
#
GID_MIN                  1000
GID_MAX                 60000
# System accounts
SYS_GID_MIN               201
SYS_GID_MAX               999

#
# If defined, this command is run when removing a user.
# It should remove any at/cron/print jobs etc. owned by
# the user to be removed (passrd as the first argument).
#
#USERDEL_CMD	/usr/sbin/userdel_local

#
# If useradd should create home directories for users by default
# On RH systems, we do. This option is overridden with the -m flag on
# useradd command line.
#
CREATE_HOME	yes

# The permission mask is initialized to this value. If not specified, 
# the permission mask will be initialized to 022.
UMASK           077

# This enables userdel to remove user groups if no members exist.
#
USERGROUPS_ENAB yes

# Use SHA512 to encrypt password.
ENCRYPT_METHOD SHA512
)"";

TEST(ConfigTest, KVConfig)
{
    auto test_filename = "/tmp/kv-config-test.conf";
    Glib::file_set_contents(test_filename, login_defs);

    auto kv_config1 = ConfigPlain::create(test_filename);
    kv_config1->set_value("PASS_MAX_DAYS", "8888");
    kv_config1->set_value("USERGROUPS_ENAB", "no");
    kv_config1->delete_key("SYS_GID_MAX");

    auto kv_config2 = ConfigPlain::create(test_filename);

    ASSERT_EQ(kv_config2->has_key("SYS_GID_MAX"), false);

    ASSERT_STREQ(kv_config2->get_value("PASS_MAX_DAYS").c_str(), "8888");
    ASSERT_EQ(kv_config2->get_integer("PASS_MAX_DAYS"), 8888);
    ASSERT_EQ(kv_config2->get_bool("TEST_BOOL_STR"), false);
    ASSERT_EQ(kv_config2->get_bool("TEST_BOOL_NUM"), true);
    ASSERT_STREQ(kv_config2->get_value("USERGROUPS_ENAB").c_str(), "no");

    ASSERT_EQ(kv_config2->is_bool("USERGROUPS_ENAB"), false);
    ASSERT_EQ(kv_config2->is_integer("CREATE_HOME"), false);
    ASSERT_EQ(kv_config2->is_integer("PASS_MAX_DAYS"), true);
}
