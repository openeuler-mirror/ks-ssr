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
#include <qt5-log-i.h>
#include <ssr-marcos.h>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QDateTime>
#include <QMap>
#include <QMetaEnum>
#include <QMutexLocker>
#include <QSettings>
#include <QString>
#include <QVector>
#include <QtDBus>
#include <atomic>

#define SSR_ACCOUNT_DBUS_OBJECT_PATH "/com/kylinsec/SSR/Account"
#define PASSWD_PATH "/etc/passwd"
#define REGISTER_USER_CMD "useradd"
#define UID_REUSE_CONTROL_PATH "/etc/uid_reuse_control.conf"
#define UID_REUSE_CONTROL_KEY "UID_REUSE_CONTROL"
#define USER_INFO_DB_TABLE_NAME "userInfo"
#define USER_INFO_DB_COLUMN1 "name"
#define USER_INFO_DB_COLUMN2 "passwd"
#define USER_INFO_INITIAL_PASSWD "123123"
#define USER_FREEZE_DB_TABLE_NAME "userFreeze"
#define USER_FREEZE_DB_COLUMN1 "name"
#define USER_FREEZE_DB_COLUMN2 "tryTimes"
#define USER_FREEZE_DB_COLUMN3 "lastTryTime"

namespace KS
{
namespace Account
{

struct Account;

/**
 * @note 前端程序在初始化时应该调用
 */
class Manager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    enum class Role
    {
        SYSADMIN,
        SECADMIN,
        SECAUDITOR
    };
    Q_ENUM(Role)

    struct Account
    {
        /**
         * @brief 当前是否是登录状态
         */
        bool m_isLogin;

        /**
         * @brief 当前用户角色，可考虑细分不同角色用户的权限
         */
        Manager::Role m_role;

        /**
         * @brief 前端程序的 pid
         */
        pid_t m_pid;
    };

private:
    Manager();
    virtual ~Manager();

public:
    static void globalInit();
    static void globDeinit();

    /**
     * @brief UID 是否可复用
     * @param enalbe 开关状态
     * @note 请注意使用 QMutexLocker 来避免多线程的问题。
     */
    void SetUidReusable(bool enalbe);

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

Q_SIGNALS:  // SIGNALS
    void PasswordChange(const QString& userName);

private:
    void initDb();
    void initUserInfoTable();
    void initUserFreezeTable();
    bool verifyPassword(const QString& userName, const QString& passwd) const;
    bool changePassword(const QString& userName, const QString& newPasswd) const;
    bool isFreeze(const QString& userName) const;
    void updateFreezeInfo(const QString& userName) const;
    void resetFreezeInfo(const QString& userName) const;



    inline pid_t getCallerPid() const
    {
        if (!calledFromDBus())
        {
            KLOG_ERROR() << "Failed to get caller pid";
            return -1;
        }
        auto dbusConn = connection();
        auto dbusMsg = message();
        auto rc_pid = dbusConn.interface()->servicePid(dbusMsg.service()).value();

        return rc_pid;
    }

    inline bool isLogin(QMap<pid_t, Account>::iterator& it)
    {
        return (m_pidToAccount.end() != it && it.value().m_isLogin);
    }

public:
    static Manager* m_accountManager;

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
    QMap<pid_t, Account> m_pidToAccount;

    QMetaEnum m_metaAccountEnum;

    /**
     * @brief 冻结时间， 3分钟，可以考虑做成配置文件中的配置项
     */
    static constexpr qint64 m_freezeLoginTimeSec = 180;

    Database* m_db;

    QMutex m_pidToAccountMutex;
    mutable QReadWriteLock m_dbMutex;
};
};  // namespace Account
};  // namespace KS