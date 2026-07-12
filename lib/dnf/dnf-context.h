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

#ifndef __KS_SSR_DNF_CONTEXT_H
#define __KS_SSR_DNF_CONTEXT_H

#include <include/ssr-marcos.h>
#include <QObject>

#if (KS_DEP_LIBDNF_VERSION < KS_VERSION_CHECK(0, 65, 0))
struct _HyGoal;
typedef struct _HyGoal* HyGoal;
#else
namespace libdnf
{
struct Goal;
}  // namespace libdnf
typedef struct libdnf::Goal* HyGoal;
#endif

namespace std
{
template <typename _Tp>
struct atomic;
}

template <typename T>
class QList;
template <typename K, typename V>
class QMap;
class QString;
struct _DnfContext;
typedef struct _DnfContext DnfContext;
struct _DnfSack;
typedef _DnfSack DnfSack;
struct _DnfState;
typedef _DnfState DnfState;
struct _GCancellable;
typedef struct _GCancellable GCancellable;
class QReadWriteLock;
class QJsonObject;
class QStringList;
class QProcess;
class QFileSystemWatcher;

enum InstallPackageAction : int;

namespace KS
{
namespace Vulnerability
{
namespace PackageManager
{
class DnfRepo;
class DnfPackage;

struct UpdatePackageResult
{
    QString output;
    bool isSuccess;
};

enum cacheStatus : int
{
    CACHE_UNAVAILABLE = -1,
    CACHE_AVAILABLE,
    CACHE_USING
};

class DnfContext : public QObject
{
    Q_OBJECT
public:
    static void globalInit();
    static void globalDeinit();
    QList<DnfRepo> getRepos();
    DnfRepo getRepoById(const QString& id);
    QList<DnfPackage> getPackagesFromRepo(DnfRepo&);
    QList<DnfPackage> getInstalledPackages();
    QList<DnfPackage> getUpgradesPackages();
    QList<DnfPackage> getLatestPackagesWithCveIds(const QStringList&);
    QList<QString> getHostArches();
    UpdatePackageResult installPackages(QList<DnfPackage>&);
    UpdatePackageResult installLocalPackages(QStringList&);
    InstallPackageAction dnfStateActionWrapper(int);
    bool installFinishedWithCancel();
    void cancelInstall();

    ::DnfContext* getDnfContext()
    {
        return m_dnfCtx;
    }

Q_SIGNALS:
    void installPercentageChanged(uint);
    void installActionChanged(InstallPackageAction, QString);
    void installAllowCancelChanged(bool);
    void installPackageProgressChanged(QString, InstallPackageAction, uint);

    // @note 此信号发出后之前所有的 dnf* 内存实例失效，需要重新获取。
    void cacheInvalidate();

public:
    static DnfContext* m_dnfCtxManager;

private:
    DnfContext();
    virtual ~DnfContext();
    void initSack();
    void getCveInfo();
    void updateCache();
    void holdCache();
    void releaseCache();

    /**
     * @brief rpm transaction to commit package(rpm install package)
     * @param pkgList packages list will be install
     * @return whether success
     *
     * @note will emit signal in this function
     */
    UpdatePackageResult transactionCommit(QList<DnfPackage>& pkgList);

    QList<DnfPackage> getPackageDeps(const DnfPackage&);

private:
    ::DnfContext* m_dnfCtx{nullptr};
    ::DnfSack* m_dnfSack{nullptr};

    // 再一次升级中可能会出现多次缓存失效的情况， 为了避免多次更新缓存， 在这里记录缓存失效次数
    // 1. 在构建缓存(initSack) 时获取一次 m_cacheStatus 在结束更新时对比， 如果一样证明在更新时没有更多的缓存失效， 则表明当前缓存最新。 如果不一致， 证明在构建缓存时又有缓存失效， 则重新构建缓存。
    // 2. 此变量同时用于记录缓存状态， 如果小于0， 则表明缓存当前不可用， 需要等待。
    std::atomic<int> m_cacheStatus{0};

    // 如果此变量为 true， 则表示缓存需要更新， 此时调用 holdCache 会阻塞。
    std::atomic<int> m_cacheNeedUpdate{0};
    std::atomic<::DnfState*> m_installState{nullptr};
    ::GCancellable* volatile m_installCancellable{nullptr};
    std::atomic<bool> m_isCancel{false};
    bool m_installFinishedWithCancel{false};
};
}  // namespace PackageManager
}  // namespace Vulnerability
}  // namespace KS

#endif