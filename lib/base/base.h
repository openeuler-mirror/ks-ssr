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

#pragma once

#include <fmt/format.h>
// #include <giomm.h>
// #include <glib/gi18n.h>

#include <qt5-log-i.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "include/ssr-error-i.h"
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"
#include "lib/base/crypto-helper.h"
#include "lib/base/error.h"
#include "lib/base/misc-utils.h"
#include "lib/base/stl-helper.h"
#include "lib/base/str-utils.h"
#include "lib/base/thread-pool.h"