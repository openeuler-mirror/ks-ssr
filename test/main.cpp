/**
 * @file          /kiran-sse-manager/test/main.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <gtest/gtest.h>
#include <zlog_ex.h>
#include "lib/base/base.h"
#include "sse-config.h"

int main(int argc, char **argv)
{
    klog_gtk3_init(std::string(), "kylinsec-session", PROJECT_NAME, "sse-tests");
    Gio::init();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
