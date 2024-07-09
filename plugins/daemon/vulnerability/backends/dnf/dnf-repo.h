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

#ifndef __KS_SSR_DNF_REPO_H
#define __KS_SSR_DNF_REPO_H

template <typename T>
class QList;
class QString;
struct _DnfRepo;
typedef _DnfRepo DnfRepo;
struct _DnfSack;
typedef _DnfSack DnfSack;
struct _DnfState;
typedef _DnfState DnfState;

namespace KS
{
namespace Vulnerability
{
namespace PackageManager
{
class DnfPackage;
class DnfRepo
{
public:
    DnfRepo();
    DnfRepo(::DnfRepo* _dnfRepo);
    DnfRepo(const DnfRepo& other);
    DnfRepo(DnfRepo&& other);
    DnfRepo& operator=(const DnfRepo& other);
    DnfRepo& operator=(DnfRepo&& other);
    virtual ~DnfRepo();
    // use g_object_unref to free return point
    ::DnfRepo* getDnfRepo();
    QString dnfRepoGetId();
    QString dnfRepoGetLocation();
    QString dnfRepoGetFilename();
    QString dnfRepoGetPackages();
    QList<QString> dnfRepoGetPublicKeys();
    // 由于无法解决前置申明枚举的问题， 所以先用int代替， 此处返回枚举应为 DnfRepoEnabled
    int dnfRepoGetEnabled();
    // 由于无法解决前置申明枚举的问题， 所以先用int代替， 此处返回枚举应为 DnfRepoKind
    int dnfRepoGetKind();
    QList<QString> dnfRepoGetExcludePackages();
    bool dnfRepoGetGpgcheck();
    bool dnfRepoGetGpgcheckMd();
    QString dnfRepoGetDescription();
    unsigned long long dnfRepoGetTimestampGenerated();
    unsigned int dnfRepoGetNSolvables();
    QString dnfRepoGetFilenameMd(const QString&);

private:
    ::DnfRepo* m_dnfRepo;
};
}  // namespace PackageManager
}  // namespace Vulnerability
}  // namespace KS
#endif