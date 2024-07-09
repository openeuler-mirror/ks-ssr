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

#include <libdnf/libdnf.h>

#include <QList>
#include <QString>
#include "dnf-package-advisory-ref.h"

#if (KS_DEP_LIBDNF_VERSION < KS_VERSION_CHECK(0, 65, 0))
#ifdef __cplusplus
extern "C"
{
#endif
    void dnf_advisoryref_free(DnfAdvisoryRef* advisoryref)
    {
        g_object_unref(advisoryref);
    }

#ifdef __cplusplus
}
#endif
#endif

namespace KS
{
namespace Vulnerability
{
namespace PackageManager
{
DnfPackageAdvisoryRef::DnfPackageAdvisoryRef()
    : m_advisoryRef(QSharedPointer<::DnfAdvisoryRef>(nullptr, dnf_advisoryref_free))
{
}

DnfPackageAdvisoryRef::DnfPackageAdvisoryRef(::DnfAdvisoryRef* advisoryRef)
    : m_advisoryRef(QSharedPointer<::DnfAdvisoryRef>(advisoryRef, dnf_advisoryref_free))
{
}

DnfPackageAdvisoryRef::DnfPackageAdvisoryRef(const DnfPackageAdvisoryRef& other)
{
    *this = other;
}

DnfPackageAdvisoryRef::DnfPackageAdvisoryRef(DnfPackageAdvisoryRef&& other)
{
    *this = std::forward<DnfPackageAdvisoryRef&&>(other);
}

DnfPackageAdvisoryRef& DnfPackageAdvisoryRef::operator=(const DnfPackageAdvisoryRef& other)
{
    m_advisoryRef = other.m_advisoryRef;
    return *this;
}

DnfPackageAdvisoryRef& DnfPackageAdvisoryRef::operator=(DnfPackageAdvisoryRef&& other)
{
    m_advisoryRef = other.m_advisoryRef;
    other.m_advisoryRef.clear();
    return *this;
}

DnfPackageAdvisoryRef::~DnfPackageAdvisoryRef()
{
}

QString DnfPackageAdvisoryRef::getId() const
{
    return dnf_advisoryref_get_id(m_advisoryRef.data());
}

int DnfPackageAdvisoryRef::getKind() const
{
    return dnf_advisoryref_get_kind(m_advisoryRef.data());
}

bool DnfPackageAdvisoryRef::isCve() const
{
    return (getKind() == DNF_REFERENCE_KIND_CVE);
}

}  // namespace PackageManager
}  // namespace Vulnerability
}  // namespace KS