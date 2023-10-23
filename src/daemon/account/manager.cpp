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

#include "manager.h"
#include <src/daemon/account_adaptor.h>

namespace KS
{
namespace Account
{

Manager* Manager::m_accountManager = nullptr;

Manager::Manager()
    : m_uidReuseConfig(new QSettings(UID_REUSE_CONTROL_PATH, QSettings::IniFormat, this)),
      m_isUidReusable(!!(m_uidReuseConfig->value(UID_REUSE_CONTROL_KEY, 0).toInt())),
      m_metaAccountEnum(QMetaEnum::fromType<Manager::Role>()),
      m_db(new Database())
{
    initDb();
    QFile file(UID_REUSE_CONTROL_PATH);
    if (!file.exists())
    {
        KLOG_ERROR() << "uid reuse control file does not exist.";
    }

    new AccountAdaptor(this);
    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if (!dbusConnection.registerObject(SSR_ACCOUNT_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "Register Account DBus object error:" << dbusConnection.lastError().message();
        abort();
    }
}

Manager::~Manager()
{
    delete m_db;
}

void Manager::globalInit()
{
    if (m_accountManager == nullptr)
    {
        m_accountManager = new Manager();
    }
}

void Manager::globDeinit()
{
    delete m_accountManager;
}

void Manager::SetUidReusable(bool enable)
{
    m_isUidReusable = enable;
    m_uidReuseConfig->setValue(UID_REUSE_CONTROL_KEY, static_cast<int>(enable));
    m_uidReuseConfig->sync();
}

bool Manager::ChangePassphrase(const QString& userName, const QString& oldPassphrase, const QString& newPassphrase)
{
    auto callerPid = getCallerPid();
    RETURN_VAL_IF_TRUE(callerPid == -1, false);

    if (!verifyPassword(userName, oldPassphrase))
    {
        KLOG_INFO() << "Password error!, failed to change passphrase";
        return false;
    }
    auto isSuccess = changePassword(userName, newPassphrase);
    emit PasswordChange(userName);
    return isSuccess;
}

bool Manager::Login(const QString& userName, const QString& passWord)
{
    auto callerPid = getCallerPid();
    RETURN_VAL_IF_TRUE(callerPid == -1, false);

    bool isSuccess = false;
    auto role_index = m_metaAccountEnum.keyToValue(userName.toLocal8Bit(), &isSuccess);
    auto role = static_cast<Role>(role_index);
    if (!isSuccess)
    {
        KLOG_ERROR() << "Unknown userName: " << userName << ", pid: " << callerPid;
        return false;
    }

    QMutexLocker locker(&m_pidToAccountMutex);
    auto it = m_pidToAccount.find(callerPid);
    if (isLogin(it))
    {
        KLOG_WARNING() << "Forward program has login, Current role: "
                       << m_metaAccountEnum.valueToKeys(static_cast<int>(it.value().m_role))
                       << ", pid: " << callerPid;
        return false;
    }

    if (isFreeze(userName))
    {
        KLOG_INFO() << userName << " has been freeze";
        return false;
    }

    if (!verifyPassword(userName, passWord))
    {
        KLOG_INFO() << "Passwd error";
        updateFreezeInfo(userName);
        return false;
    }
    resetFreezeInfo(userName);
    m_pidToAccount.insert(callerPid, {true, role, callerPid});
    return true;
}

bool Manager::Logout()
{
    auto callerPid = getCallerPid();
    RETURN_VAL_IF_TRUE(callerPid == -1, false);

    QMutexLocker locker(&m_pidToAccountMutex);
    auto it = m_pidToAccount.find(callerPid);
    if (!isLogin(it))
    {
        KLOG_ERROR() << "Maybe its internal error, Please login before logout, pid: " << callerPid;
        return false;
    }
    it.value().m_isLogin = false;
    return true;
}

void Manager::initDb()
{
    constexpr const char* getUserInfoTables = "SELECT * "
                                              "FROM sqlite_master "
                                              "WHERE type='table' "
                                              "AND name ='" USER_INFO_DB_TABLE_NAME "';";
    constexpr const char* getUserFreezeTables = "SELECT * "
                                                "FROM sqlite_master "
                                                "WHERE type='table' "
                                                "AND name ='" USER_FREEZE_DB_TABLE_NAME "';";
    SqlDataType res{};
    if (!m_db->exec(getUserInfoTables, &res))
    {
        KLOG_ERROR() << "Failed to get table: " USER_INFO_DB_TABLE_NAME;
        abort();
    }
    if (res.isEmpty())
    {
        KLOG_INFO() << "Db table: init table: " USER_INFO_DB_TABLE_NAME;
        initUserInfoTable();
    }

    if (!m_db->exec(getUserFreezeTables, &res))
    {
        KLOG_ERROR() << "Failed to get table: " USER_FREEZE_DB_TABLE_NAME;
        abort();
    }
    if (res.isEmpty())
    {
        KLOG_DEBUG() << "Db table: init table: " USER_FREEZE_DB_TABLE_NAME;
        initUserFreezeTable();
    }
}

void Manager::initUserInfoTable()
{
    constexpr const char* createUserInfoTables = "CREATE table " USER_INFO_DB_TABLE_NAME
                                                 " (" USER_INFO_DB_COLUMN1 " vchar, " USER_INFO_DB_COLUMN2 " vchar);";
    if (!m_db->exec(createUserInfoTables))
    {
        KLOG_ERROR() << "Failed to create table: " USER_INFO_DB_TABLE_NAME;
        abort();
    }
    for (auto i = 0; i < m_metaAccountEnum.keyCount(); i++)
    {
        const QString insertUserInfo("insert into " USER_INFO_DB_TABLE_NAME " values ('%1', '%2');");
        auto userName = m_metaAccountEnum.key(i);
        if (!m_db->exec(insertUserInfo.arg(userName).arg(USER_INFO_INITIAL_PASSWD)))
        {
            KLOG_ERROR() << "Failed insert user info: " << userName;
            abort();
        }
    }
}

void Manager::initUserFreezeTable()
{
    constexpr const char* createUserFreezeInfoTables = "CREATE table " USER_FREEZE_DB_TABLE_NAME
                                                       " (" USER_FREEZE_DB_COLUMN1 " vchar, " USER_FREEZE_DB_COLUMN2 " int, " USER_FREEZE_DB_COLUMN3 " int);";
    if (!m_db->exec(createUserFreezeInfoTables))
    {
        KLOG_ERROR() << "Failed to create table: " USER_INFO_DB_TABLE_NAME;
        abort();
    }
    for (auto i = 0; i < m_metaAccountEnum.keyCount(); i++)
    {
        const QString insertUserFreeze("insert into " USER_FREEZE_DB_TABLE_NAME " values ('%1', %2, %3)");
        auto userName = m_metaAccountEnum.key(i);
        if (!m_db->exec(insertUserFreeze.arg(userName).arg(0).arg(0)))
        {
            KLOG_ERROR() << "Failed insert user Freeze: " << userName;
            abort();
        }
    }
}

bool Manager::verifyPassword(const QString& userName, const QString& passwd) const
{
    constexpr const char* rawCmd = " SELECT " USER_INFO_DB_COLUMN2
                                   " FROM " USER_INFO_DB_TABLE_NAME
                                   " WHERE " USER_INFO_DB_COLUMN1 " = '%1';";
    QString sqlCmd{rawCmd};
    SqlDataType res;
    KLOG_DEBUG() << "sql cmd: " << sqlCmd.arg(userName);
    QReadLocker locker(&m_dbMutex);
    if (!m_db->exec(sqlCmd.arg(userName), &res))
    {
        KLOG_ERROR() << "Failed to access db";
        return false;
    }
    return (!res.isEmpty() && res[0][0].toString() == passwd);
}

bool Manager::isFreeze(const QString& userName) const
{
    constexpr const char* queryUserFreeze = " SELECT *"
                                            " FROM " USER_FREEZE_DB_TABLE_NAME
                                            " WHERE " USER_FREEZE_DB_COLUMN1 "='%1';";
    SqlDataType res{};
    QReadLocker locker(&m_dbMutex);
    if (!m_db->exec(QString(queryUserFreeze).arg(userName), &res))
    {
        KLOG_ERROR() << "Failed query table: " USER_FREEZE_DB_TABLE_NAME;
        return true;
    }
    locker.unlock();
    if (res.isEmpty())
    {
        KLOG_ERROR() << "Internal error: db";
        return true;
    }
    auto tryLoginTimes = res[0][1].toInt();
    auto lastLoginTime = res[0][2].toLongLong();
    auto currentTime = QDateTime::currentDateTime().toSecsSinceEpoch();
    // 当 tryLoginTimes >= 5 且 lastLoginTime + m_freezeLoginTimeSec < currentTime时
    // 表明此帐号曾经被冻结，但是冻结时间已过，所以应该重置登录次数
    if (tryLoginTimes >= 5 && lastLoginTime + m_freezeLoginTimeSec < currentTime)
    {
        resetFreezeInfo(userName);
        return false;
    }
    return (tryLoginTimes >= 5 && lastLoginTime + m_freezeLoginTimeSec > currentTime);
}

void Manager::updateFreezeInfo(const QString& userName) const
{
    constexpr const char* updateFreeze = " UPDATE " USER_FREEZE_DB_TABLE_NAME
                                         " SET " USER_FREEZE_DB_COLUMN2 " = " USER_FREEZE_DB_COLUMN2 " + 1, " USER_FREEZE_DB_COLUMN3 " = %1"
                                         " WHERE " USER_FREEZE_DB_COLUMN1 " = '%2'";

    QWriteLocker locker(&m_dbMutex);
    if (!m_db->exec(QString(updateFreeze).arg(QDateTime::currentDateTime().toSecsSinceEpoch()).arg(userName)))
    {
        KLOG_ERROR() << "Failed to Update " USER_FREEZE_DB_TABLE_NAME;
        abort();
    }
}

void Manager::resetFreezeInfo(const QString& userName) const
{
    constexpr const char* resetFreeze = " UPDATE " USER_FREEZE_DB_TABLE_NAME
                                        " SET " USER_FREEZE_DB_COLUMN2 " = 0, " USER_FREEZE_DB_COLUMN3 " = 0"
                                        " WHERE " USER_FREEZE_DB_COLUMN1 " = '%1'";
    QWriteLocker locker(&m_dbMutex);
    if (!m_db->exec(QString(resetFreeze).arg(userName)))
    {
        KLOG_ERROR() << "Failed to reset " USER_FREEZE_DB_TABLE_NAME;
    }
}

bool Manager::changePassword(const QString& userName, const QString& newPasswd) const
{
    constexpr const char* rawCmd = " UPDATE " USER_INFO_DB_TABLE_NAME
                                   " SET " USER_INFO_DB_COLUMN2 " = '%1'"
                                   " WHERE " USER_INFO_DB_COLUMN1 " = '%2'";
    QString sqlCmd{rawCmd};
    KLOG_DEBUG() << "sql cmd: " << sqlCmd.arg(newPasswd).arg(userName);
    QWriteLocker locker(&m_dbMutex);
    return m_db->exec(sqlCmd.arg(newPasswd).arg(userName));
}
};  // namespace Account
};  // namespace KS