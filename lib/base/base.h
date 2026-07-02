/**
 * @file          /kiran-ssr-manager/lib/base/base.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <fmt/format.h>
#include <giomm.h>

#include <gtk3-log-i.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "lib/base/crypto-helper.h"
#include "lib/base/def.h"
#include "lib/base/error.h"
#include "lib/base/file-utils.h"
#include "lib/base/glib2-utils.h"
#include "lib/base/stl-helper.h"
#include "lib/base/str-utils.h"
#include "lib/base/thread-pool.h"
#include "ssr-i.h"