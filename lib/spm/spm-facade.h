/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#ifndef __PACKAGE_MANGER_H
#define __PACKAGE_MANGER_H

#ifdef WITH_LIBDNF
#include "dnf-headers.h"
#endif

namespace KS
{
namespace Vulnerability
{
namespace PackageManager
{
using Context
#ifdef WITH_LIBDNF
    = DnfContext;
#endif

using Repo
#ifdef WITH_LIBDNF
    = DnfRepo;
#endif

using Package
#ifdef WITH_LIBDNF
    = DnfPackage;
#endif

using PackageAdvisory
#ifdef WITH_LIBDNF
    = DnfPackageAdvisory;
#endif

using PackageAdvisoryRef
#ifdef WITH_LIBDNF
    = DnfPackageAdvisoryRef;
#endif

}  // namespace PackageManager
}  // namespace Vulnerability
}  // namespace KS

#endif