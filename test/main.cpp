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

    auto test_result = RUN_ALL_TESTS();
    auto loop = Glib::MainLoop::create();

    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    timeout.connect_seconds([loop]() -> bool {
        loop->quit();
        return false;
    },
                            1);
    loop->run();
    return test_result;
}
