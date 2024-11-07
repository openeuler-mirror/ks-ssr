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

#ifndef __KS_SSR_DNF_PACKAGE_ADVISORY_H
#define __KS_SSR_DNF_PACKAGE_ADVISORY_H

#include <ssr-marcos.h>
#include <QSharedPointer>
#include <QString>

#if (KS_DEP_LIBDNF_VERSION >= KS_VERSION_CHECK(0, 15, 0))
namespace libdnf
{
struct Advisory;
}
typedef struct libdnf::Advisory DnfAdvisory;
#else
struct _DnfAdvisory;
typedef struct _DnfAdvisory DnfAdvisory;
#endif

template <typename T>
class QSharedPointer;
template <typename T>
class QList;
class QString;

namespace KS
{
namespace Vulnerability
{
namespace PackageManager
{
class DnfPackageAdvisoryRef;

class DnfPackageAdvisory
{
public:
    struct DnfAdvisoryPkg
    {
        QString name;
        QString evr;
        QString arch;
        QString fileName;
    };

public:
    DnfPackageAdvisory();
    DnfPackageAdvisory(::DnfAdvisory* advisory);
    DnfPackageAdvisory(const DnfPackageAdvisory& other);
    DnfPackageAdvisory(DnfPackageAdvisory&& other);
    DnfPackageAdvisory& operator=(const DnfPackageAdvisory& other);
    DnfPackageAdvisory& operator=(DnfPackageAdvisory&& other);
    virtual ~DnfPackageAdvisory();
    QList<DnfPackageAdvisoryRef> getRefs() const;
    QList<DnfAdvisoryPkg> getPkgList() const;
    QString getTitle() const;
    QString getId() const;
    int getKind() const;
    bool isSecurity() const;

private:
    QSharedPointer<::DnfAdvisory> m_advisory;
};
}  // namespace PackageManager
}  // namespace Vulnerability
}  // namespace KS
#endif