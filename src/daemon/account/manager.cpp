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
#define RSA_KEY_LENGTH 512

namespace KS
{
namespace Account
{
const Manager* Manager::m_accountManager = nullptr;

Manager::Manager()
    : m_metaAccountEnum(QMetaEnum::fromType<AccountRole>()),
      m_uidReuseConfig(new QSettings(UID_REUSE_CONTROL_PATH, QSettings::IniFormat, this)),
      m_isUidReusable(!!(m_uidReuseConfig->value(UID_REUSE_CONTROL_KEY, 0).toInt())),
      m_db(new Database()),
      m_dbusServerWatcher(new QDBusServiceWatcher(this))

{
    initDatabase();
    QFile file(UID_REUSE_CONTROL_PATH);
    if (!file.exists())
    {
        KLOG_ERROR() << "uid reuse control file does not exist.";
    }

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

void Manager::SetUidReusable(bool enabled)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = m_accountManager->getRole(calledUniqueName);
    if (role == KS::Account::Manager::AccountRole::unknown_account)
    {
        KLOG_ERROR() << "Failed to set uid reusable, Permission denied";
        SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, "Permission Denied", false);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, enabled ? "Enable uid reuse" : "Disable uid reuse");
    m_isUidReusable = enabled;
    m_uidReuseConfig->setValue(UID_REUSE_CONTROL_KEY, static_cast<int>(enabled));
    m_uidReuseConfig->sync();
}

bool Manager::GetUidReusable()
{
    return m_isUidReusable;
}

void Manager::disableMultiFactorAuthState()
{
    auto msg = QDBusMessage::createMethodCall(KAD_MANAGER_DBUS_NAME,
                                              KAD_MANAGER_DBUS_OBJECT_PATH,
                                              KAD_MANAGER_DBUS_INTERFACE_NAME,
                                              "SetAuthTypeEnabled");
    msg.setArguments({static_cast<int>(KAD_AUTH_APPLICATION_LOGIN), false});
    auto replyMsg = QDBusConnection::systemBus().call(msg);
    if (replyMsg.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << "Failed to call dbus method SetAuthTypeEnabled: " << replyMsg.errorMessage();
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    // auth        include        kiran-authentication-service
    QRegExp authIncKiranAuthService(REGEXP_PAM_AUTH_INC_KIRANAUTHSERVICE);

    // account     include        kiran-authentication-service
    QRegExp accountIncKiranAuthService(REGEXP_PAM_ACCOUNT_INC_KIRANAUTHSERVICE);
    QFile systemAuth(PAM_SYSTEM_PATH);
    if (!systemAuth.open(QIODevice::ReadWrite))
    {
        KLOG_ERROR() << "Failed to open system auth file";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    QString systemAuthContent = systemAuth.readAll();
    if (!systemAuthContent.contains(authIncKiranAuthService) ||
        !systemAuthContent.contains(accountIncKiranAuthService))
    {
        KLOG_ERROR() << "Multi-Factor Authentication is disable, skip";
        return;
    }
    systemAuthContent.remove(authIncKiranAuthService);
    systemAuthContent.remove(accountIncKiranAuthService);
    systemAuth.resize(0);
    systemAuth.write(systemAuthContent.toLocal8Bit());
    systemAuth.flush();
    systemAuth.close();

    QFile kiranAuth(PAM_KIRAN_PATH);
    if (!kiranAuth.open(QIODevice::ReadWrite))
    {
        KLOG_ERROR() << "Failed to open kiran auth file";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    QString kiranAuthContent = kiranAuth.readAll();
    QRegularExpression multiWay(REGEXP_MULTI_WAY_AUTH);
    auto multiWayMatch = multiWay.match(kiranAuthContent);
    if (!multiWayMatch.hasMatch())
    {
        KLOG_ERROR() << "Failed to match Multi-Way authentication";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    // ((#)(auth  [success=done ignore=2 default=bad authinfo_unavail=die] pam_kiran_authentication.so doauth))
    kiranAuthContent.replace(multiWayMatch.captured(0), multiWayMatch.captured(2));

    QRegularExpression multiFactor(REGEXP_MULTI_FACTOR_AUTH);
    auto multiFactorMatch = multiFactor.match(kiranAuthContent);
    if (!multiFactorMatch.hasMatch())
    {
        KLOG_ERROR() << "Failed to match Multi-Factor authentication";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    kiranAuthContent.replace(multiFactorMatch.captured(0), "#" + multiFactorMatch.captured(2));
    kiranAuth.seek(0);
    kiranAuth.write(kiranAuthContent.toLocal8Bit());
    kiranAuth.flush();
}

void Manager::enableMultiFactorAuthState()
{
    auto msg = QDBusMessage::createMethodCall(KAD_MANAGER_DBUS_NAME,
                                              KAD_MANAGER_DBUS_OBJECT_PATH,
                                              KAD_MANAGER_DBUS_INTERFACE_NAME,
                                              "SetAuthTypeEnabled");
    msg.setArguments({static_cast<int>(KAD_AUTH_APPLICATION_LOGIN), true});
    auto replyMsg = QDBusConnection::systemBus().call(msg);
    if (replyMsg.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << "Failed to call dbus method SetAuthTypeEnabled: " << replyMsg.errorMessage();
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    // auth        include        kiran-authentication-service
    QRegExp authIncKiranAuthService(REGEXP_PAM_AUTH_INC_KIRANAUTHSERVICE);

    // account     include        kiran-authentication-service
    QRegExp accountIncKiranAuthService(REGEXP_PAM_ACCOUNT_INC_KIRANAUTHSERVICE);
    QFile systemAuth(PAM_SYSTEM_PATH);
    if (!systemAuth.open(QIODevice::ReadWrite))
    {
        KLOG_ERROR() << "Failed to open system auth file";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    QString systemAuthContent = systemAuth.readAll();
    if (systemAuthContent.contains(authIncKiranAuthService) ||
        systemAuthContent.contains(accountIncKiranAuthService))
    {
        KLOG_INFO() << "Multi-Factor Authentication is enable, skip";
        return;
    }

    // (auth[ ]+sufficient[ ]+pam_unix.so)
    QRegularExpression authSufPamUnixDotSO(REGEXP_PAM_AUTH_SUF_PAMUNIXDOTSO);
    // (account[ ]+required[ ]+pam_unix.so)
    QRegularExpression accountReqPamUnixDotSO(REGEXP_PAM_ACCOUNT_REQ_PAMUNIXDOTSO);
    auto matchAuth = authSufPamUnixDotSO.match(systemAuthContent);
    if (!matchAuth.hasMatch())
    {
        KLOG_ERROR() << "Failed to match pam auth";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    systemAuthContent.replace(authSufPamUnixDotSO, PAM_KIRAN_AUTH_CONFIG + matchAuth.captured(1));

    auto matchAccount = accountReqPamUnixDotSO.match(systemAuthContent);
    if (!matchAccount.hasMatch())
    {
        KLOG_ERROR() << "Failed to match pam account";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    systemAuthContent.replace(accountReqPamUnixDotSO, PAM_KIRAN_ACCOUNT_CONFIG + matchAccount.captured(1));
    systemAuth.resize(0);
    systemAuth.write(systemAuthContent.toLocal8Bit());
    systemAuth.flush();

    QFile kiranAuth(PAM_KIRAN_PATH);
    if (!kiranAuth.open(QIODevice::ReadWrite))
    {
        KLOG_ERROR() << "Failed to open kiran auth file";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    QString kiranAuthContent = kiranAuth.readAll();
    QRegularExpression multiWay(REGEXP_MULTI_WAY_AUTH);
    auto multiWayMatch = multiWay.match(kiranAuthContent);
    if (!multiWayMatch.hasMatch())
    {
        KLOG_ERROR() << "Failed to match Multi-Way authentication";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    KLOG_DEBUG() << multiWayMatch.captured(0);
    KLOG_DEBUG() << multiWayMatch.captured(1);
    KLOG_DEBUG() << multiWayMatch.captured(2);
    // ((#)(auth  [success=done ignore=2 default=bad authinfo_unavail=die] pam_kiran_authentication.so doauth))
    kiranAuthContent.replace(multiWayMatch.captured(0), "#" + multiWayMatch.captured(2));

    QRegularExpression multiFactor(REGEXP_MULTI_FACTOR_AUTH);
    auto multiFactorMatch = multiFactor.match(kiranAuthContent);
    if (!multiFactorMatch.hasMatch())
    {
        KLOG_ERROR() << "Failed to match Multi-Factor authentication";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE, this->message());
    }
    kiranAuthContent.replace(multiFactorMatch.captured(0), multiFactorMatch.captured(2));
    kiranAuth.seek(0);
    kiranAuth.write(kiranAuthContent.toLocal8Bit());
    kiranAuth.flush();
}

void Manager::SetMultiFactorAuthState(bool enabled)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = m_accountManager->getRole(calledUniqueName);
    if (role == KS::Account::Manager::AccountRole::unknown_account)
    {
        KLOG_ERROR() << "Failed to set Multi-Factor Authentication state, Permission denied";
        SSR_LOG(role, Log::Manager::LogType::ACCOUNT, "Permission Denied", false);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    SSR_LOG(role, Log::Manager::LogType::ACCOUNT, enabled ? "Enable Multi-Factor Authentication" : "Disable Multi-Factor Authentication");
    if (enabled)
    {
        enableMultiFactorAuthState();
    }
    else
    {
        disableMultiFactorAuthState();
    }
}

bool Manager::getMultiFactorAuthState()
{
    auto msg = QDBusMessage::createMethodCall(KAD_MANAGER_DBUS_NAME,
                                              KAD_MANAGER_DBUS_OBJECT_PATH,
                                              KAD_MANAGER_DBUS_INTERFACE_NAME,
                                              "GetAuthTypeByApp");
    msg.setArguments({static_cast<int>(KAD_AUTH_APPLICATION_LOGIN)});
    auto replyMsg = QDBusConnection::systemBus().call(msg);
    if (replyMsg.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << "Failed to call dbus method GetAuthTypeByApp: " << replyMsg.errorMessage();
        return false;
    }
    QDBusPendingReply<QList<int>> reply(replyMsg);
    auto ret = reply.value();
    if (!ret.contains(static_cast<int>(KAD_AUTH_TYPE_UKEY)))
    {
        return false;
    }
    msg = QDBusMessage::createMethodCall(KAD_MANAGER_DBUS_NAME,
                                         KAD_MANAGER_DBUS_OBJECT_PATH,
                                         KAD_MANAGER_DBUS_INTERFACE_NAME,
                                         "GetAuthTypeEnabled");
    msg.setArguments({static_cast<int>(KAD_AUTH_TYPE_UKEY)});
    replyMsg = QDBusConnection::systemBus().call(msg);
    if (replyMsg.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << "Failed to call dbus method GetAuthTypeEnabled: " << replyMsg.errorMessage();
        return false;
    }
    if (replyMsg.arguments().at(0).toBool() == false)
    {
        return false;
    }

    QFile systemAuth(PAM_SYSTEM_PATH);
    // auth        requisite      pam_faillock.so  preauth audit deny=3...
    QRegExp authReqFailLockDotSo(REGEXP_PAM_AUTH_REQ_FAILLOCKDOTSO_REGEXP);

    // auth        include        kiran-authentication-service
    QRegExp authIncKiranAuthService(REGEXP_PAM_AUTH_INC_KIRANAUTHSERVICE);

    // account     include        kiran-authentication-service
    QRegExp accountIncKiranAuthService(REGEXP_PAM_ACCOUNT_INC_KIRANAUTHSERVICE);
    if (!systemAuth.open(QIODevice::ReadOnly))
    {
        KLOG_ERROR() << "Failed to open system auth file";
        return false;
    }
    QString systemAuthContent = systemAuth.readAll();
    if (!systemAuthContent.contains(authReqFailLockDotSo))
    {
        KLOG_INFO() << "Auth pam_faillock.so must be requisite";
        return false;
    }
    if (!systemAuthContent.contains(authIncKiranAuthService))
    {
        KLOG_INFO() << "System auth doesn't contain kiran-authentication-server";
        return false;
    }
    if (!systemAuthContent.contains(accountIncKiranAuthService))
    {
        KLOG_INFO() << "System account doesn't contain kiran-authentication-server";
        return false;
    }
    return true;
}

bool Manager::GetMultiFactorAuthState()
{
    return m_multiFactorAuthState;
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
        SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, QString("Failed to change %1's passphrase, unique name: %2, role: %3").arg(userName).arg(calledUniqueName).arg(roleName), false);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    if (!verifyPassword(userName, oldPassphrase))
    {
        KLOG_INFO() << "Password error!, failed to change passphrase";
        SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, "Change password", false);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_PASSWORD_ERROR, this->message());
    }
    if (oldPassphrase == newPassphrase)
    {
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_BE_DIFF_NEW_PASSWORD, this->message());
    }
    auto isSuccess = changePassword(userName, newPassphrase);
    emit PasswordChanged(userName);
    SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, "Change password");
    return isSuccess;
}

bool Manager::Login(const QString& userName, const QString& passWord)
{
    auto callerUnique = DBusHelper::getCallerUniqueName(this);
    bool isSuccess = false;
    auto role_index = m_metaAccountEnum.keyToValue(userName.toLocal8Bit(), &isSuccess);
    auto role = static_cast<AccountRole>(role_index);

    if (!isSuccess)
    {
        KLOG_ERROR() << "Unknown meta user name: " << userName << ", Unique name: " << callerUnique;
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_UNKNOWN_ACCOUNT, this->message());
    }
    m_dbusServerWatcher->addWatchedService(callerUnique);

    QWriteLocker locker(&m_clientMutex);
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
        SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, "Login, but this account has been freeze", false);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_BE_FREEZE, this->message());
    }

    if (!verifyPassword(userName, passWord))
    {
        KLOG_INFO() << "Passwd error";
        updateFreezeInfo(userName);
        SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, "Login, Passwd error", false);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(false, SSRErrorCode::ERROR_ACCOUNT_PASSWORD_ERROR, this->message());
    }
    resetFreezeInfo(userName);
    m_clients.insert(callerUnique, {true, role, DBusHelper::getCallerPid(this)});
    SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, "Login");
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
    SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, "Logout");
    return true;
}

void Manager::initDatabase()
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
        return;
    }
    if (res.isEmpty())
    {
        KLOG_INFO() << "Db table: init table: " USER_INFO_DB_TABLE_NAME;
        initUserInfoTable();
    }

    if (!m_db->exec(getUserFreezeTables, &res))
    {
        KLOG_ERROR() << "Failed to get table: " USER_FREEZE_DB_TABLE_NAME;
        return;
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
        return;
    }
    for (auto i = 0; i < m_metaAccountEnum.keyCount(); i++)
    {
        const QString insertUserInfo("insert into " USER_INFO_DB_TABLE_NAME " values ('%1', '%2');");
        auto userName = m_metaAccountEnum.key(i);
        auto encrypterdPassword = CryptoHelper::aesEncrypt(USER_INFO_INITIAL_PASSWD);
        if (!m_db->exec(insertUserInfo.arg(userName).arg(encrypterdPassword)))
        {
            KLOG_ERROR() << "Failed insert user info: " << userName;
            return;
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
        return;
    }
    for (auto i = 0; i < m_metaAccountEnum.keyCount(); i++)
    {
        const QString insertUserFreeze("insert into " USER_FREEZE_DB_TABLE_NAME " values ('%1', %2, %3)");
        auto userName = m_metaAccountEnum.key(i);
        if (!m_db->exec(insertUserFreeze.arg(userName).arg(0).arg(0)))
        {
            KLOG_ERROR() << "Failed insert user Freeze: " << userName;
            return;
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
    if (res.isEmpty())
    {
        KLOG_WARNING() << "This account dont have password, username: " << userName;
        return false;
    }

    auto decryptedPassword = CryptoHelper::rsaDecryptString(m_rsaPrivateKey, passwd);
    auto currentPassword = CryptoHelper::aesDecrypt(res[0][0].toString());
    return decryptedPassword == currentPassword;
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
    QWriteLocker locker(&m_dbMutex);
    // rsa -> raw text
    auto rsaDecryptedPassword = CryptoHelper::rsaDecryptString(m_rsaPrivateKey, newPasswd);
    // raw text -> aes
    auto aesEncryptedPassword = CryptoHelper::aesEncrypt(rsaDecryptedPassword);
    return m_db->exec(sqlCmd.arg(aesEncryptedPassword).arg(userName));
}
};  // namespace Account
};  // namespace KS
