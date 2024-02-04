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
#include <kiran-authentication-service/kas-authentication-i.h>
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
#define PAM_SYSTEM_PATH "/etc/pam.d/system-auth"
#define PAM_KIRAN_PATH "/etc/pam.d/kiran-authentication-service"
#define PAM_KIRAN_AUTH_CONFIG "auth        include        kiran-authentication-service\n"
#define PAM_KIRAN_ACCOUNT_CONFIG "account     include       kiran-authentication-service\n"
#define REGEXP_PAM_AUTH_REQ_FAILLOCKDOTSO_REGEXP R"(auth[ ]+requisite[ ]+pam_faillock.so)"
#define REGEXP_PAM_AUTH_INC_KIRANAUTHSERVICE R"(auth[ ]+include[ ]+kiran-authentication-service\n)"
#define REGEXP_PAM_ACCOUNT_INC_KIRANAUTHSERVICE R"(account[ ]+include[ ]+kiran-authentication-service\n)"
#define REGEXP_PAM_AUTH_SUF_PAMUNIXDOTSO R"((auth[ ]+sufficient[ ]+pam_unix.so))"
#define REGEXP_PAM_ACCOUNT_REQ_PAMUNIXDOTSO R"((account[ ]+required[ ]+pam_unix.so))"
#define REGEXP_MULTI_WAY_AUTH R"((.*)(auth[ ]+\[success=done ignore=2 default=bad authinfo_unavail=die\][ ]+pam_kiran_authentication.so[ ]+doauth))"
#define REGEXP_MULTI_FACTOR_AUTH R"((.*)(auth[ ]+\[success=2 default=bad\][ ]+pam_kiran_authentication.so[ ]+doauth))"
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
    m_multiFactorAuthState = getMultiFactorAuthState();
    KLOG_INFO() << "Multi-Factor Authentication state: " + QString(m_multiFactorAuthState ? "enable" : "disable");
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
        SSR_LOG_ERROR(Log::Manager::LogType::ACCOUNT, "Permission Denied", calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::ACCOUNT, enabled ? "Enable uid reuse" : "Disable uid reuse", calledUniqueName);
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
    // 开启多因子认证时会把 ukey 之外的所有认证方式关闭， 所以关闭多因子认证时需还原设置
    enableAuthType({
        static_cast<int>(KAD_AUTH_TYPE_FINGERPRINT),
        static_cast<int>(KAD_AUTH_TYPE_FACE),
        static_cast<int>(KAD_AUTH_TYPE_UKEY),
        static_cast<int>(KAD_AUTH_TYPE_FINGERVEIN),
        static_cast<int>(KAD_AUTH_TYPE_IRIS)
    });
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
    // 关闭所有认证方式， 然后再启用 ukey 认证方式
    disableAuthType({
        static_cast<int>(KAD_AUTH_TYPE_FINGERPRINT),
        static_cast<int>(KAD_AUTH_TYPE_FACE),
        static_cast<int>(KAD_AUTH_TYPE_UKEY),
        static_cast<int>(KAD_AUTH_TYPE_FINGERVEIN),
        static_cast<int>(KAD_AUTH_TYPE_IRIS)
    });
    auto msg = QDBusMessage::createMethodCall(KAD_MANAGER_DBUS_NAME,
                                              KAD_MANAGER_DBUS_OBJECT_PATH,
                                              KAD_MANAGER_DBUS_INTERFACE_NAME,
                                              "SetAuthTypeEnabled");
    msg.setArguments({static_cast<int>(KAD_AUTH_TYPE_UKEY), true});
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

bool Manager::checkPassword(const QString& password, const QString& userName)
{
    // 不允许包含用户名 CaseInsensitive : 区分大小写
    RETURN_VAL_IF_TRUE(password.contains(userName, Qt::CaseInsensitive), false);
    // 至少包含一个小写字母，一个大写字母，一个数字，一个特殊字符中的两种，最少八位
    QRegularExpression regex("^(?![\\d]+$)(?![a-z]+$)(?![A-Z]+$)(?![^\\da-zA-Z]+$).{8,16}$");
    auto match = regex.match(password);
    return match.hasMatch();
}

void Manager::SetMultiFactorAuthState(bool enabled)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = m_accountManager->getRole(calledUniqueName);
    if (role == KS::Account::Manager::AccountRole::unknown_account)
    {
        KLOG_ERROR() << "Failed to set Multi-Factor Authentication state, Permission denied";
        SSR_LOG_ERROR(Log::Manager::LogType::ACCOUNT, "Permission Denied", calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    SSR_LOG_SUCCESS(
        Log::Manager::LogType::ACCOUNT,
        enabled ? "Enable Multi-Factor Authentication" : "Disable Multi-Factor Authentication",
        calledUniqueName);
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

void Manager::disableAuthType(QList<int> authTypes)
{
    for (const auto authType : authTypes)
    {
        auto msg = QDBusMessage::createMethodCall(KAD_MANAGER_DBUS_NAME,
                                              KAD_MANAGER_DBUS_OBJECT_PATH,
                                              KAD_MANAGER_DBUS_INTERFACE_NAME,
                                              "SetAuthTypeEnabled");
        msg.setArguments({authType, false});
        auto replyMsg = QDBusConnection::systemBus().call(msg);
        if (replyMsg.type() != QDBusMessage::ReplyMessage)
        {
            KLOG_WARNING() << "Failed to disable auth, type: " << authType << ", error msg: " << replyMsg.errorMessage();
        }
    }
}

void Manager::enableAuthType(QList<int> authTypes)
{
    for (const auto authType : authTypes)
    {
        auto msg = QDBusMessage::createMethodCall(KAD_MANAGER_DBUS_NAME,
                                              KAD_MANAGER_DBUS_OBJECT_PATH,
                                              KAD_MANAGER_DBUS_INTERFACE_NAME,
                                              "SetAuthTypeEnabled");
        msg.setArguments({authType, true});
        auto replyMsg = QDBusConnection::systemBus().call(msg);
        if (replyMsg.type() != QDBusMessage::ReplyMessage)
        {
            KLOG_WARNING() << "Failed to enable auth, type: " << authType << ", error msg: " << replyMsg.errorMessage();
        }
    }
}
};  // namespace Account
};  // namespace KS
