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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#ifndef __KS_SSR_DNF_PACKAGE_ADVISORY_REF_H
#define __KS_SSR_DNF_PACKAGE_ADVISORY_REF_H

#include <ssr-marcos.h>
#include <QSharedPointer>

#if (KS_DEP_LIBDNF_VERSION >= KS_VERSION_CHECK(0, 15, 0))
namespace libdnf
{
struct AdvisoryRef;
}  // namespace libdnf
typedef struct libdnf::AdvisoryRef DnfAdvisoryRef;
#else
struct _DnfAdvisoryRef;
typedef struct _DnfAdvisoryRef DnfAdvisoryRef;
#endif
template <typename T>
class QList;
class QString;

namespace KS
{
namespace Vulnerability
{
namespace PackageManager
{
class DnfPackageAdvisoryRef
{
public:
    DnfPackageAdvisoryRef();
    DnfPackageAdvisoryRef(::DnfAdvisoryRef* advisoryRef);
    DnfPackageAdvisoryRef(const DnfPackageAdvisoryRef& other);
    DnfPackageAdvisoryRef(DnfPackageAdvisoryRef&& other);
    DnfPackageAdvisoryRef& operator=(const DnfPackageAdvisoryRef& other);
    DnfPackageAdvisoryRef& operator=(DnfPackageAdvisoryRef&& other);
    virtual ~DnfPackageAdvisoryRef();
    QString getId() const;
    int getKind() const;
    bool isCve() const;

private:
    QSharedPointer<::DnfAdvisoryRef> m_advisoryRef;
};
}  // namespace PackageManager
}  // namespace Vulnerability
}  // namespace KS
#endif