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
#include <zlog_ex.h>
#include "config.h"
#include "lib/base/base.h"

int main(int argc, char **argv)
{
    klog_gtk3_init(std::string(), "kylinsec-session", PROJECT_NAME, "br-tests");
    Gio::init();
    testing::InitGoogleTest(&argc, argv);

    auto test_result = RUN_ALL_TESTS();
    auto loop = Glib::MainLoop::create();

    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    timeout.connect_seconds([loop]() -> bool
                            {
                                loop->quit();
                                return false;
                            },
                            1);
    loop->run();
    return test_result;
}
