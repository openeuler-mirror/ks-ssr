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

#include "src/daemon/account/manager.h"
#include <qt5-log-i.h>
#include <src/daemon/account_adaptor.h>
#include <src/daemon/common/dbus-helper.h>
#include <ssr-i.h>
#include <ssr-marcos.h>
#include <QDBusServiceWatcher>
#include <QSettings>
#include <QtDBus>
#include <iostream>
#include "include/ssr-marcos.h"
#include "lib/base/crypto-helper.h"
#include "lib/base/error.h"
#include "src/daemon/log/manager.h"

#define SSR_ACCOUNT_DBUS_OBJECT_PATH "/com/kylinsec/SSR/Account"
#define PASSWD_PATH "/etc/passwd"
#define REGISTER_USER_CMD "useradd"
#define USER_INFO_DB_TABLE_NAME "userInfo"
#define USER_INFO_DB_COLUMN1 "name"
#define USER_INFO_DB_COLUMN2 "role"
#define USER_INFO_DB_COLUMN3 "passwd"
#define USER_INFO_DB_COLUMN4 "tryTimes"
#define USER_INFO_DB_COLUMN5 "lastTryTime"
#define USER_INFO_DB_POSITION_NAME 0
#define USER_INFO_DB_POSITION_ROLE 1
#define USER_INFO_DB_POSITION_PASSWD 2
#define USER_INFO_DB_POSITION_TRY_TIMES 3
#define USER_INFO_DB_POSITION_LAST_TRY_TIME 4
#define USER_INFO_INITIAL_PASSWD "kylin.123"
#define RSA_KEY_LENGTH 512
namespace KS
{
namespace Account
{
const Manager* Manager::m_accountManager = nullptr;

Manager::Manager()
    : m_metaAccountEnum(QMetaEnum::fromType<AccountRole>()),
      m_db(new Database()),
      m_dbusServerWatcher(new QDBusServiceWatcher(this))

{
    initDatabase();

    KS::CryptoHelper::generateRsaKey(RSA_KEY_LENGTH, m_rsaPrivateKey, m_rsaPublicKey);

    new AccountAdaptor(this);
    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if (!dbusConnection.registerObject(SSR_ACCOUNT_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "Register Account DBus object error:" << dbusConnection.lastError().message();
    }

    m_dbusServerWatcher->setConnection(dbusConnection);
    m_dbusServerWatcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    connect(m_dbusServerWatcher, &QDBusServiceWatcher::serviceUnregistered, [this](const QString& service)
            {
                this->m_dbusServerWatcher->removeWatchedService(service);
                KLOG_INFO() << "The front program has exit, clean data. Unique Name: " << service;
                QWriteLocker locker(&(this->m_clientMutex));
                auto it = this->m_clients.find(service);
                if (it == this->m_clients.end())
                {
                    return;
                }
                this->m_clients.erase(it);
            });
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

bool Manager::checkPassword(const QString& password, const QString& userName)
{
    // 不允许包含用户名 CaseInsensitive : 区分大小写
    RETURN_VAL_IF_TRUE(password.contains(userName, Qt::CaseInsensitive), false);
    // 至少包含一个小写字母，一个大写字母，一个数字，一个特殊字符中的两种，最少八位
    QRegularExpression regex("^(?![\\d]+$)(?![a-z]+$)(?![A-Z]+$)(?![^\\da-zA-Z]+$).{8,16}$");
    auto match = regex.match(password);
    return match.hasMatch();
}

bool Manager::ChangePassphrase(const QString& userName, const QString& oldPassphrase, const QString& newPassphrase)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = m_accountManager->getRole(calledUniqueName);
    auto roleName = KS::Account::Manager::m_accountManager->m_metaAccountEnum.valueToKey(static_cast<int>(role));
    if (role == KS::Account::Manager::AccountRole::unknown_account ||
        userName != roleName)
    {
        KLOG_ERROR() << "Failed to change " << userName << "'s passphrase, unique name: "
                     << calledUniqueName << ", role: " << roleName;
        SSR_LOG_ERROR(
            Log::Manager::LogType::ACCOUNT,
            tr("Failed to change %1's passphrase, unique name: %2, actor role: %3")
                .arg(userName)
                .arg(calledUniqueName)
                .arg(roleName),
            calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    if (!verifyPassword(userName, oldPassphrase))
    {
        KLOG_INFO() << "Password error!, failed to change passphrase";
        SSR_LOG_ERROR(Log::Manager::LogType::ACCOUNT, tr("Change password"), calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_PASSWORD_ERROR, this->message());
    }
    if (!checkPassword(CryptoHelper::rsaDecryptString(m_rsaPrivateKey, newPassphrase), userName))
    {
        SSR_LOG_ERROR(Log::Manager::LogType::ACCOUNT, tr("Change password failed."), calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_CHECK_PASSWORD_FAILED, this->message());
    }

    if (CryptoHelper::rsaDecryptString(m_rsaPrivateKey, oldPassphrase) ==
        CryptoHelper::rsaDecryptString(m_rsaPrivateKey, newPassphrase))
    {
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_BE_DIFF_NEW_PASSWORD, this->message());
    }
    auto isSuccess = changePassword(userName, newPassphrase);
    emit PasswordChanged(userName);
    SSR_LOG_SUCCESS(Log::Manager::LogType::ACCOUNT, tr("Change password"), calledUniqueName);
    return isSuccess;
}

bool Manager::Login(const QString& userName, const QString& passWord)
{
    auto callerUnique = DBusHelper::getCallerUniqueName(this);
    auto role = getRoleFromDB(userName);
    Log::Log log{userName, role, QDateTime::currentDateTime(), Log::Manager::LogType::ACCOUNT, false, ""};
    if (role == AccountRole::unknown_account)
    {
        KLOG_ERROR() << "Unknown user name: " << userName << ", Unique name: " << callerUnique;
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_UNKNOWN_ACCOUNT, this->message());
    }
    m_dbusServerWatcher->addWatchedService(callerUnique);
    auto it = m_clients.find(callerUnique);

    if (isLogin(it))
    {
        KLOG_WARNING() << "Forward program has login, Current role: "
                       << m_metaAccountEnum.valueToKeys(static_cast<int>(it.value().role))
                       << ", Unique name: " << callerUnique;
        return false;
    }

    if (isFreeze(userName))
    {
        KLOG_INFO() << userName << " has been freeze";
        log.logMsg = tr("Failed to login, because this account has been freeze");
        Log::Manager::writeLog(log);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_BE_FREEZE, this->message());
    }

    if (!verifyPassword(userName, passWord))
    {
        KLOG_INFO() << "Passwd error";
        updateFreezeInfo(userName);
        log.logMsg = tr("Failed to login, Passwd error");
        Log::Manager::writeLog(log);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_PASSWORD_ERROR, this->message());
    }
    QWriteLocker locker(&m_clientMutex);
    resetFreezeInfo(userName);
    m_clients.insert(callerUnique, {true, role, userName, DBusHelper::getCallerPid(this)});
    locker.unlock();
    log.result = true;
    log.logMsg = tr("Login");
    Log::Manager::writeLog(log);
    return true;
}

bool Manager::Logout()
{
    auto callerUnique = DBusHelper::getCallerUniqueName(this);
    auto role = m_accountManager->getRole(callerUnique);
    if (role == KS::Account::Manager::AccountRole::unknown_account)
    {
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_UNKNOWN_ACCOUNT, this->message());
    }

    QReadLocker locker(&m_clientMutex);
    auto it = m_clients.find(callerUnique);
    if (!isLogin(it))
    {
        KLOG_ERROR() << "Maybe its internal error, Please login before logout, Unique name: " << callerUnique;
        return false;
    }
    it.value().isLogin = false;
    SSR_LOG_SUCCESS(Log::Manager::LogType::ACCOUNT, tr("Logout"), callerUnique);
    return true;
}

void Manager::createUser(const QString& userName, const QString& role, const QString& password)
{
    constexpr const char* insertUserInfo = "insert into " USER_INFO_DB_TABLE_NAME
                                           " values ('%1', '%2', '%3', '%4', '%5');";
    if (!m_db->exec(QString(insertUserInfo).arg(userName).arg(role).arg(password).arg(0).arg(0)))
    {
        KLOG_ERROR() << "Failed to add user: " << userName;
    }
}

void Manager::initDatabase()
{
    constexpr const char* getUserInfoTables = "SELECT * "
                                              "FROM sqlite_master "
                                              "WHERE type='table' "
                                              "AND name ='" USER_INFO_DB_TABLE_NAME "';";
    SqlDataType res{};
    if (!m_db->exec(getUserInfoTables, &res))
    {
        KLOG_ERROR() << "Failed to get table: " USER_INFO_DB_TABLE_NAME;
        return;
    }
    if (res.isEmpty())
    {
        KLOG_INFO() << "Db table: init table: " USER_INFO_DB_TABLE_NAME;
        initUserInfoTable();
    }
}

void Manager::initUserInfoTable()
{
    constexpr const char* createUserInfoTables = "CREATE table " USER_INFO_DB_TABLE_NAME  // clang-format off
                                                 " ( " USER_INFO_DB_COLUMN1 " vchar, "    // clang-format off
                                                       USER_INFO_DB_COLUMN2 " vchar, "    // clang-format off
                                                       USER_INFO_DB_COLUMN3 " vchar, "    // clang-format off
                                                       USER_INFO_DB_COLUMN4 " int, "      // clang-format off
                                                       USER_INFO_DB_COLUMN5 " int "       // clang-format off
                                                 ");";
    if (!m_db->exec(createUserInfoTables))
    {
        KLOG_ERROR() << "Failed to create table: " USER_INFO_DB_TABLE_NAME;
        return;
    }
    for (auto i = 0; i < m_metaAccountEnum.keyCount(); i++)
    {
        auto userName = m_metaAccountEnum.key(i);
        auto encryptedPassword = CryptoHelper::aesEncrypt(USER_INFO_INITIAL_PASSWD);
        createUser(userName, userName, encryptedPassword);
    }
}

bool Manager::verifyPassword(const QString& userName, const QString& passwd) const
{
    constexpr const char* rawCmd = " SELECT " USER_INFO_DB_COLUMN3
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
    if (res.isEmpty())
    {
        KLOG_WARNING() << "This account dont have password, username: " << userName;
        return false;
    }

    auto decryptedPassword = CryptoHelper::rsaDecryptString(m_rsaPrivateKey, passwd);
    auto currentPassword = CryptoHelper::aesDecrypt(res[0][0].toString());
    return decryptedPassword == currentPassword;
}

Manager::AccountRole Manager::getRoleFromDB(const QString& userName) const
{
    constexpr const char* queryAccountInfo = " SELECT " USER_INFO_DB_COLUMN2
                                             " FROM " USER_INFO_DB_TABLE_NAME
                                             " WHERE " USER_INFO_DB_COLUMN1 "='%1';";
    SqlDataType res{};
    QReadLocker locker(&m_dbMutex);
    if (!m_db->exec(QString(queryAccountInfo).arg(userName), &res))
    {
        KLOG_ERROR() << "Failed query table: " USER_INFO_DB_TABLE_NAME;
        return AccountRole::unknown_account;
    }
    locker.unlock();
    if (res.isEmpty())
    {
        KLOG_INFO() << QString("User %1 does not exist").arg(userName);
        return AccountRole::unknown_account;
    }
    return static_cast<AccountRole>(m_metaAccountEnum.keyToValue(res[0][0].toString().toLocal8Bit()));
}

bool Manager::isFreeze(const QString& userName) const
{
    constexpr const char* queryUserFreeze = " SELECT *"
                                            " FROM " USER_INFO_DB_TABLE_NAME
                                            " WHERE " USER_INFO_DB_COLUMN1 "='%1';";
    SqlDataType res{};
    QReadLocker locker(&m_dbMutex);
    if (!m_db->exec(QString(queryUserFreeze).arg(userName), &res))
    {
        KLOG_ERROR() << "Failed query table: " USER_INFO_DB_TABLE_NAME;
        return true;
    }
    locker.unlock();
    if (res.isEmpty())
    {
        KLOG_ERROR() << "Internal error: db";
        return true;
    }
    auto tryLoginTimes = res[0][USER_INFO_DB_POSITION_TRY_TIMES].toInt();
    auto lastLoginTime = res[0][USER_INFO_DB_POSITION_LAST_TRY_TIME].toLongLong();
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
    constexpr const char* updateFreeze = " UPDATE " USER_INFO_DB_TABLE_NAME
                                         " SET " USER_INFO_DB_COLUMN4 " = " USER_INFO_DB_COLUMN4 " + 1, " USER_INFO_DB_COLUMN5 " = %1"
                                         " WHERE " USER_INFO_DB_COLUMN1 " = '%2'";

    QWriteLocker locker(&m_dbMutex);
    if (!m_db->exec(QString(updateFreeze).arg(QDateTime::currentDateTime().toSecsSinceEpoch()).arg(userName)))
    {
        KLOG_ERROR() << "Failed to Update FreezeInfo!";
    }
}

void Manager::resetFreezeInfo(const QString& userName) const
{
    constexpr const char* resetFreeze = " UPDATE " USER_INFO_DB_TABLE_NAME
                                        " SET " USER_INFO_DB_COLUMN4 " = 0, " USER_INFO_DB_COLUMN5 " = 0"
                                        " WHERE " USER_INFO_DB_COLUMN1 " = '%1'";
    QWriteLocker locker(&m_dbMutex);
    if (!m_db->exec(QString(resetFreeze).arg(userName)))
    {
        KLOG_ERROR() << "Failed to reset " USER_INFO_DB_TABLE_NAME;
    }
}

bool Manager::changePassword(const QString& userName, const QString& newPasswd) const
{
    constexpr const char* rawCmd = " UPDATE " USER_INFO_DB_TABLE_NAME
                                   " SET " USER_INFO_DB_COLUMN3 " = '%1'"
                                   " WHERE " USER_INFO_DB_COLUMN1 " = '%2'";
    QString sqlCmd{rawCmd};
    QWriteLocker locker(&m_dbMutex);
    // rsa -> raw text
    auto rsaDecryptedPassword = CryptoHelper::rsaDecryptString(m_rsaPrivateKey, newPasswd);
    // raw text -> aes
    auto aesEncryptedPassword = CryptoHelper::aesEncrypt(rsaDecryptedPassword);
    return m_db->exec(sqlCmd.arg(aesEncryptedPassword).arg(userName));
}
};  // namespace Account
};  // namespace KS
