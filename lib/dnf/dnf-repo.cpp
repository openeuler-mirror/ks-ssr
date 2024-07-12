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

#include "dnf-repo.h"
#include <glib.h>
#include <libdnf/libdnf.h>
#include <QList>
#include "dnf-context.h"

// 在同时引用 glib 头文件和 QObject 时， 注意 QObject 得在 glib 之后包含， 不然会出现编译错误。
#include <qt5-log-i.h>

namespace KS
{
namespace Vulnerability
{
namespace PackageManager
{
DnfRepo::DnfRepo()
    : m_dnfRepo(nullptr)
{
}

DnfRepo::DnfRepo(::DnfRepo* _dnfRepo)
{
    m_dnfRepo = (::DnfRepo*)g_object_ref((gpointer)_dnfRepo);
}

DnfRepo::DnfRepo(const DnfRepo& other)
{
    *this = other;
}

DnfRepo::DnfRepo(DnfRepo&& other)
{
    *this = std::forward<DnfRepo&&>(other);
}

DnfRepo& DnfRepo::operator=(const DnfRepo& other)
{
    m_dnfRepo = other.m_dnfRepo;
    g_object_ref((gpointer)m_dnfRepo);
    return *this;
}

DnfRepo& DnfRepo::operator=(DnfRepo&& other)
{
    m_dnfRepo = other.m_dnfRepo;
    other.m_dnfRepo = nullptr;
    return *this;
}

DnfRepo::~DnfRepo()
{
    g_object_unref(m_dnfRepo);
}

::DnfRepo* DnfRepo::getDnfRepo()
{
    return (::DnfRepo*)g_object_ref((gpointer)m_dnfRepo);
}

QString DnfRepo::dnfRepoGetId()
{
    return QString(dnf_repo_get_id(m_dnfRepo));
}

QString DnfRepo::dnfRepoGetLocation()
{
    return QString(dnf_repo_get_location(m_dnfRepo));
}

QString DnfRepo::dnfRepoGetFilename()
{
    return QString(dnf_repo_get_filename(m_dnfRepo));
}

QString DnfRepo::dnfRepoGetPackages()
{
    return QString(dnf_repo_get_packages(m_dnfRepo));
}

QList<QString> DnfRepo::dnfRepoGetPublicKeys()
{
    QList<QString> ret{};
    auto publicKeys = dnf_repo_get_public_keys(m_dnfRepo);

    while (*publicKeys)
    {
        ret.append(*publicKeys);
        publicKeys++;
    }
    return ret;
}

int DnfRepo::dnfRepoGetEnabled()
{
    return static_cast<unsigned int>(DnfRepoEnabled(dnf_repo_get_enabled(m_dnfRepo)));
}

int DnfRepo::dnfRepoGetKind()
{
    return static_cast<unsigned int>(dnf_repo_get_kind(m_dnfRepo));
}

QList<QString> DnfRepo::dnfRepoGetExcludePackages()
{
    QList<QString> ret{};
    auto excludePkgs = dnf_repo_get_exclude_packages(m_dnfRepo);
    while (*excludePkgs)
    {
        ret.append(*excludePkgs);
        excludePkgs++;
    }
    return ret;
}

bool DnfRepo::dnfRepoGetGpgcheck()
{
    return dnf_repo_get_gpgcheck(m_dnfRepo);
}

bool DnfRepo::dnfRepoGetGpgcheckMd()
{
    return dnf_repo_get_gpgcheck_md(m_dnfRepo);
}

QString DnfRepo::dnfRepoGetDescription()
{
    return dnf_repo_get_description(m_dnfRepo);
}

unsigned long long DnfRepo::dnfRepoGetTimestampGenerated()
{
    return dnf_repo_get_timestamp_generated(m_dnfRepo);
}

unsigned int DnfRepo::dnfRepoGetNSolvables()
{
    return dnf_repo_get_n_solvables(m_dnfRepo);
}

QString DnfRepo::dnfRepoGetFilenameMd(const QString& mdKind)
{
    return dnf_repo_get_filename_md(m_dnfRepo, mdKind.toLatin1().data());
}

}  // namespace PackageManager
}  // namespace Vulnerability
}  // namespace KS