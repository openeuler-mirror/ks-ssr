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

#pragma once

#include <daemon-accounts-i.h>
#include <daemon-plugin-i.h>
#include <lib/base/database.h>
#include <ssr-i.h>
#include <QDBusContext>
#include <QMetaEnum>
#include <QMutex>
#include <QReadWriteLock>

class QSettings;
class QMutexLocker;
class QDBusServiceWatcher;

namespace KS
{
namespace Accounts
{
struct Account;

class Manager : public QObject,
                public IDaemonAccounts,
                public QDBusContext
{
    Q_OBJECT

public:
    struct Account
    {
        /**
         * @brief 当前是否是登录状态
         */
        bool isLogin;

        /**
         * @brief 当前用户角色，可考虑细分不同角色用户的权限
         */
        AccountRole role;

        /**
         * @brief 用户名
         */
        QString name;

        /**
         * @brief 前端程序的 pid
         */
        pid_t pid;
    };

public:
    Manager();
    virtual ~Manager();

    bool ChangePassphrase(const QString& userName, const QString& oldPassphrase, const QString& newPassphrase);
    bool Login(const QString& userName, const QString& passWord);
    // 获取当前连接会话的登录角色
    int GetLoginRole();
    QString GetRoleName(int role);
    bool Logout();

public:  // PROPERTIES
    Q_PROPERTY(QString RSAPublicKey READ rsaPublicKey)
    QString rsaPublicKey() const
    {
        return m_rsaPublicKey;
    };

    virtual AccountRole getRole(const QString& dbusUniqueName) const
    {
        QReadLocker locker(&m_clientMutex);
        auto it = m_clients.find(dbusUniqueName);
        if (it == m_clients.end())
        {
            KLOG_WARNING() << "Unknown dbus id: " << dbusUniqueName;
            return AccountRole::ACCOUNT_ROLE_NOACCOUNT;
        }
        return it->role;
    }

    virtual AccountRole getRole(pid_t dbusPid) const
    {
        QReadLocker locker(&m_clientMutex);
        for (const auto& client : m_clients)
        {
            if (client.pid == dbusPid)
            {
                return client.role;
            }
        }
        KLOG_WARNING() << "Unknown dbus id: " << dbusPid;
        return AccountRole::ACCOUNT_ROLE_NOACCOUNT;
    }

    virtual QString getUserName(const QString& dbusUniqueName) const
    {
        QReadLocker locker(&m_clientMutex);
        auto it = m_clients.find(dbusUniqueName);
        if (it == m_clients.end())
        {
            KLOG_WARNING() << "Unknown dbus id: " << dbusUniqueName;
            return "unknown";
        }
        return it->name;
    }

    virtual QString getUserName(pid_t dbusPid) const
    {
        QReadLocker locker(&m_clientMutex);
        for (const auto& client : m_clients)
        {
            if (client.pid == dbusPid)
            {
                return client.name;
            }
        }
        KLOG_WARNING() << "Unknown dbus id: " << dbusPid;
        return "unknown";
    }

    virtual QString accountRoleEnum2Str(AccountRole role) const;
    virtual AccountRole accountRoleStr2Enum(const QString& roleStr) const;

Q_SIGNALS:  // SIGNALS
    void PasswordChanged(const QString& user_name);

private:
    void createUser(const QString& userName, const QString& role, const QString& password);
    void initDatabase();
    void initUserInfoTable();
    bool verifyPassword(const QString& userName, const QString& passwd) const;
    bool changePassword(const QString& userName, const QString& newPasswd) const;
    bool isFreeze(const QString& userName) const;
    AccountRole getRoleFromDB(const QString& userName) const;
    void updateFreezeInfo(const QString& userName) const;
    void resetFreezeInfo(const QString& userName) const;
    // 密码复杂度检测
    bool checkPassword(const QString& password, const QString& userName);

    inline bool isLogin(QMap<QString, Account>::iterator& it)
    {
        return (m_clients.end() != it && it.value().isLogin);
    }

private:
    /**
     * @brief 键为前端程序的 pid ，值为前端程序对应账户的实例化结构体
     */
    QMap<QString, Account> m_clients;

    /**
     * @brief 冻结时间， 3分钟，可以考虑做成配置文件中的配置项
     */
    static constexpr qint64 m_freezeLoginTimeSec = 180;

    Database* m_db;

    mutable QReadWriteLock m_clientMutex;
    mutable QReadWriteLock m_dbMutex;
    QDBusServiceWatcher* m_dbusServerWatcher;
    QString m_rsaPublicKey;  // property
    QString m_rsaPrivateKey;
};
};  // namespace Accounts
};  // namespace KS
