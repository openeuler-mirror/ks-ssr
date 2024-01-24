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

#include <lib/base/database.h>
#include <QDBusContext>
#include <QMetaEnum>
#include <QMutex>
#include <QReadWriteLock>

class QSettings;
class QMutexLocker;
class QDBusServiceWatcher;

namespace KS
{
namespace Account
{
struct Account;

class Manager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    // 此枚举类型为与前台传入参数保持一致，使用小写定义枚举
    enum class AccountRole
    {
        sysadm = (1 << 0),
        secadm = (1 << 1),
        audadm = (1 << 2),
        unknown_account = (1 << 3)
    };
    Q_ENUM(AccountRole)
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

private:
    Manager();
    virtual ~Manager();

public:
    static void globalInit();
    static void globDeinit();

    /**
     * @brief UID 是否可复用
     * @param enabled 开关状态
     * @note 请注意使用 QMutexLocker 来避免多线程的问题。
     */
    void SetUidReusable(bool enabled);

    /**
     * @brief 修改密码
     * @param passphrase 新密码
     * @return 是否修改成功
     */
    bool ChangePassphrase(const QString& userName, const QString& oldPassphrase, const QString& newPassphrase);

    /**
     * @brief 登录函数
     * @param userName 用户名
     * @param passWord 密码
     * @return 是否登录成功
     */
    bool Login(const QString& userName, const QString& passWord);

    /**
     * @brief 注销函数
     * @return 是否注销成功
     */
    bool Logout();

    bool GetUidReusable();
    void SetMultiFactorAuthState(bool enabled);
    bool GetMultiFactorAuthState();

public:  // PROPERTIES
    Q_PROPERTY(QString RSAPublicKey READ rsaPublicKey)
    QString rsaPublicKey() const
    {
        return m_rsaPublicKey;
    };

    AccountRole getRole(QString dbusUniqueName) const
    {
        QReadLocker locker(&m_clientMutex);
        auto it = m_clients.find(dbusUniqueName);
        if (it == m_clients.end())
        {
            KLOG_WARNING() << "Unknown dbus id: " << dbusUniqueName;
            return AccountRole::unknown_account;
        }
        return it->role;
    }

    AccountRole getRole(pid_t dbusPid) const
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
        return AccountRole::unknown_account;
    }

    QString getUserName(QString dbusUniqueName) const
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

    QString getUserName(pid_t dbusPid) const
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
    QMetaEnum m_metaAccountEnum;
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
    bool getMultiFactorAuthState();
    void disableMultiFactorAuthState();
    void enableMultiFactorAuthState();
    // 密码复杂度检测
    bool checkPassword(const QString& password, const QString& userName);

    inline bool isLogin(QMap<QString, Account>::iterator& it)
    {
        return (m_clients.end() != it && it.value().isLogin);
    }

public:
    static const Manager* m_accountManager;

private:
    /**
     * @brief uid 是否可复用的配置文件， 路径默认是 UID_REUSE_CONTROL_PATH
     */
    QSettings* m_uidReuseConfig;

    /**
     * @brief UID 是否可复用
     */
    bool m_isUidReusable;

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
    bool m_multiFactorAuthState;
};
};  // namespace Account
};  // namespace KS