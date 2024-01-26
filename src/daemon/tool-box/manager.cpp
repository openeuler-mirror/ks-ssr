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

#include "src/daemon/log/manager.h"
#include <grp.h>
#include <pwd.h>
#include <selinux/selinux.h>
#include <src/daemon/account/manager.h>
#include <src/daemon/log/manager.h>
#include <src/daemon/log/message.h>
#include <src/daemon/tool_box_adaptor.h>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QReadWriteLock>
#include "include/ssr-error-i.h"
#include "include/ssr-i.h"
#include "lib/base/database.h"
#include "lib/base/error.h"
#include "src/daemon/common/dbus-helper.h"
#include "src/daemon/tool-box/manager.h"
#include "src/daemon/tool-box/realtime-alert.h"

#define SHRED_PATH "/usr/bin/shred"
#define SHRED_ARG_1 "-f"
#define SHRED_ARG_2 "-u"
#define USERDEL_PATH "/usr/sbin/userdel"
#define USERDEL_ARG_1 "-r"
#define SED_PATH "/usr/bin/sed"
#define DISABLE_SELINUX "-i -re 's/SELINUX=(enforcing|permissive|disabled)/SELINUX=disabled/' /etc/selinux/config"
#define ENABLE_SELINUX "-i -re 's/SELINUX=(enforcing|permissive|disabled)/SELINUX=enforcing/' /etc/selinux/config"
#define PASSWD_FILE "/etc/passwd"

#define FILE_SIGN_TABLE "fileSign"
#define FILE_SIGN_COLUMN1 "filePath"

#define FILE_SHRED_TABLE "fileShred"
#define FILE_SHRED_COLUMN1 "filePath"

namespace KS
{
namespace ToolBox
{
Manager* Manager::m_toolBoxManager = nullptr;

void Manager::globalInit()
{
    if (m_toolBoxManager == nullptr)
    {
        m_toolBoxManager = new Manager();
    }
}

void Manager::globalDeinit()
{
    delete m_toolBoxManager;
}

Manager::Manager()
    : m_osUserNameMutex(new QReadWriteLock()),
      m_userNameWatcher(new QFileSystemWatcher(QStringList(PASSWD_FILE), this)),
      m_realTimeAlert(new RealTimeAlert()),
      m_db(new Database())
{
    initDatabase();
    new ToolBoxAdaptor(this);
    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if (!dbusConnection.registerObject(SSR_TOOL_BOX_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "Register ToolBox DBus object error:" << dbusConnection.lastError().message();
    }
    getAllUsers();
    connect(m_userNameWatcher, &QFileSystemWatcher::fileChanged, [this](const QString&)
            {
                // linux 创建和删除用户时， 可能会存在 /etc/passwd 文件删除又创建的情况
                this->m_userNameWatcher->removePath(PASSWD_FILE);
                if (QFile::exists(PASSWD_FILE))
                {
                    this->m_userNameWatcher->addPath(PASSWD_FILE);
                }
                this->getAllUsers();
                emit this->UserChanged();
            });
}

void Manager::initDatabase()
{
    constexpr const char* getFileSignTables = "SELECT * "
                                              "FROM sqlite_master "
                                              "WHERE type='table' "
                                              "AND name ='" FILE_SIGN_TABLE "';";

    constexpr const char* createFileSignTables = "CREATE table " FILE_SIGN_TABLE
                                                 " ( " FILE_SIGN_COLUMN1 " vchar UNIQUE"
                                                 ");";

    constexpr const char* getFileShredTables = "SELECT * "
                                               "FROM sqlite_master "
                                               "WHERE type='table' "
                                               "AND name ='" FILE_SHRED_TABLE "';";

    constexpr const char* createFileShredTables = "CREATE table " FILE_SHRED_TABLE
                                                  " ( " FILE_SHRED_COLUMN1 " vchar UNIQUE"
                                                  " );";

    SqlDataType res{};
    if (!m_db->exec(getFileSignTables, &res))
    {
        KLOG_ERROR() << "Failed to get table: " FILE_SIGN_TABLE;
        return;
    }
    if (res.isEmpty())
    {
        if (!m_db->exec(createFileSignTables, &res))
        {
            KLOG_ERROR() << "Failed to create table: " FILE_SIGN_TABLE;
        }
    }

    if (!m_db->exec(getFileShredTables, &res))
    {
        KLOG_ERROR() << "Failed to get table: " FILE_SHRED_TABLE;
        return;
    }
    if (res.isEmpty())
    {
        if (!m_db->exec(createFileShredTables, &res))
        {
            KLOG_ERROR() << "Failed to create table: " FILE_SHRED_TABLE;
        }
    }
}

void Manager::SetAccessControlStatus(bool enable)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set access control status, permission denied"),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    QProcess process{};
    process.setProgram(SED_PATH);
    QStringList arg{"-i"};
    arg.append("-re");
    arg.append(enable
                   ? R"({s#SELINUX=(enforcing|permissive|disabled)#SELINUX=enforcing#})"
                   : R"({s/SELINUX=(enforcing|permissive|disabled)/SELINUX=disabled/})");
    arg.append("/etc/selinux/config");
    process.setArguments(arg);
    process.start();
    process.waitForFinished();
    if (process.exitCode() != 0)
    {
        KLOG_ERROR() << "Failed to execute cmd: " << process.program()
                     << " " << process.arguments().join(' ')
                     << ", exitcode: " << process.exitCode()
                     << ", output: " << process.readAllStandardOutput();
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set access control status"),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_ACCESS_CONTROL, this->message());
        return;
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("set access control status to %1").arg(enable ? "enable" : "disable"),
                    calledUniqueName);
}

QString Manager::GetSecurityContext(const QString& filePath)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to get security context, permission denied"),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    char* context = nullptr;
    if (getfilecon(filePath.toLocal8Bit(), &context) == -1)
    {
        KLOG_ERROR() << "Failed to get " << filePath
                     << "selinux context, error message: " << strerror(errno);
        freecon(context);
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to get %1 selinux context").arg(filePath),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_TOOL_BOX_FAILED_GET_SECURITY_CONTEXT, this->message());
        return QString();
    }
    QString rs(context);
    freecon(context);
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Get %1 selinux context").arg(filePath),
                    calledUniqueName);
    return rs;
}

void Manager::SetSecurityContext(const QString& filePath, const QString& SecurityContext)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set security context, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    if (setfilecon(filePath.toLocal8Bit(), SecurityContext.toLocal8Bit()) == -1)
    {
        KLOG_ERROR() << "Failed to set " << filePath
                     << " selinux context: " << SecurityContext
                     << "error message: " << strerror(errno);
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set %1 selinux context, error msg: %2").arg(filePath).arg(strerror(errno)),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_SECURITY_CONTEXT, this->message());
        return;
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Set %1 selinux context to: %2").arg(filePath).arg(SecurityContext),
                    calledUniqueName);
}

void Manager::ShredFile(const QStringList& filePath)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    auto userName = Account::Manager::m_accountManager->getUserName(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to shred file, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    Log::Log log = {userName, role, QDateTime::currentDateTime(),
                    Log::Manager::LogType::TOOL_BOX, false, tr("Shred files %1").arg(filePath.join(' '))};
    // TODO : 粉碎文件夹不生效
    auto cmd = getProcess(log, SHRED_PATH, QStringList{SHRED_ARG_1, SHRED_ARG_2} << filePath);
    cmd->start();
    RemoveFileFromFileShred(filePath);
}

void Manager::RemoveUser(const QStringList& userNames)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    auto userName = Account::Manager::m_accountManager->getUserName(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to remove user, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    Log::Log log = {userName, role, QDateTime::currentDateTime(),
                    Log::Manager::LogType::TOOL_BOX, false, tr("Remove users %1").arg(userNames.join(' '))};
    for (const auto& userName : userNames)
    {
        auto cmd = getProcess(log, USERDEL_PATH, QStringList{USERDEL_ARG_1} << userName);
        cmd->start();
    }
}

bool Manager::GetAccessStatus()
{
    return static_cast<bool>(is_selinux_enabled());
}

QStringList Manager::GetFileListFromFileSign()
{
    constexpr const char* getFileFromFileSign = "select " FILE_SIGN_COLUMN1 " from " FILE_SIGN_TABLE;

    SqlDataType res{};
    QStringList ret{};
    if (!m_db->exec(getFileFromFileSign, &res))
    {
        KLOG_ERROR() << "Failed to get files from " << FILE_SIGN_TABLE;
    }
    for (const auto& res_row : res)
    {
        ret << res_row[0].toString();
    }
    return ret;
}

void Manager::AddFileToFileSign(const QStringList& file_list)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to add files to SignFile list, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    constexpr const char* insertFileToFileSign = "insert OR IGNORE into " FILE_SIGN_TABLE
                                                 " values ('%1');";
    QString insertFileToFileSignDBCmd{insertFileToFileSign};
    if (!m_db->exec(insertFileToFileSignDBCmd.arg(file_list.join("'), ('"))))
    {
        KLOG_ERROR() << "Failed to add files: " << file_list;
    }
    emit FileSignListChanged();
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Add file to SignFile list: %1").arg(file_list.join(' ')),
                    calledUniqueName);
}

void Manager::RemoveFileFromFileSign(const QStringList& file_list)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to remove file from SignFile list, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    constexpr const char* removeFileFromFileSign = "delete from " FILE_SIGN_TABLE
                                                   " where " FILE_SIGN_COLUMN1
                                                   " in ('%1');";
    QString removeFileToFileSignDBCmd{removeFileFromFileSign};
    if (!m_db->exec(removeFileToFileSignDBCmd.arg(file_list.join("', '"))))
    {
        KLOG_ERROR() << "Failed to remove files: " << file_list;
    }
    emit FileSignListChanged();
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Remove file from SignFile list: %1").arg(file_list.join(' ')),
                    calledUniqueName);
}

QStringList Manager::GetFileListFromFileShred()
{
    constexpr const char* getFileFromFileSign = "select " FILE_SHRED_COLUMN1 " from " FILE_SHRED_TABLE;

    SqlDataType res{};
    QStringList ret{};
    if (!m_db->exec(getFileFromFileSign, &res))
    {
        KLOG_ERROR() << "Failed to get files from " << FILE_SHRED_TABLE;
    }
    for (const auto& res_row : res)
    {
        ret << res_row[0].toString();
    }
    return ret;
}

void Manager::AddFileToFileShred(const QStringList& file_list)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to add files to ShredFile list, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    constexpr const char* insertFileToFileShred = "insert OR IGNORE into " FILE_SHRED_TABLE
                                                  " values ('%1');";
    QString insertFileToFileShredDBCmd{insertFileToFileShred};
    if (!m_db->exec(insertFileToFileShredDBCmd.arg(file_list.join("'), ('"))))
    {
        KLOG_ERROR() << "Failed to add files: " << file_list;
    }
    emit FileShredListChanged();
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Add file to ShredFile list: %1").arg(file_list.join(' ')),
                    calledUniqueName);
}

void Manager::RemoveFileFromFileShred(const QStringList& file_list)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to remove file from ShredFile list, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    constexpr const char* removeFileFromFileShred = "delete from " FILE_SHRED_TABLE
                                                    " where " FILE_SHRED_COLUMN1
                                                    " in ('%1');";
    QString removeFileToFileShredDBCmd{removeFileFromFileShred};
    if (!m_db->exec(removeFileToFileShredDBCmd.arg(file_list.join("', '"))))
    {
        KLOG_ERROR() << "Failed to remove files: " << file_list;
    }
    emit FileShredListChanged();
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Remove file from ShredFile list: %1").arg(file_list.join(' ')),
                    calledUniqueName);
}

QString Manager::GetAllUsers()
{
    getAllUsers();
    QReadLocker locker(m_osUserNameMutex);
    return m_osUserInfoJson;
}

void Manager::getAllUsers(const QString&)
{
    QJsonArray arr;
    QStringList managerUserList;
    const QString managerGroup("wheel");
    struct group* groupInfo;
    setgrent();
    while ((groupInfo = getgrent()) != nullptr)
    {
        if (groupInfo->gr_name != managerGroup)
        {
            continue;
        }
        char** members = groupInfo->gr_mem;
        while (*members != nullptr)
        {
            QJsonObject obj;
            managerUserList.append(*members);
            obj.insert("name", *members);
            obj.insert("type", OsUserType::USER_TYPE_MANAGER);
            arr.append(obj);
            members++;
        }
        break;
    }
    struct passwd* pw;
    setpwent();  // 重置密码文件的读取位置到开头
    while ((pw = getpwent()) != nullptr)
    {
        if (managerUserList.contains(pw->pw_name))
        {
            continue;
        }
        QJsonObject obj;
        obj.insert("name", pw->pw_name);
        if (pw->pw_uid < 1000)
        {
            obj.insert("type", OsUserType::USER_TYPE_MANAGER);
        }
        else
        {
            obj.insert("type", OsUserType::USER_TYPE_NORMAL);
        }
        arr.append(obj);
    }
    endpwent();  // 关闭密码文件
    QJsonDocument doc(arr);
    QWriteLocker locker(m_osUserNameMutex);
    m_osUserInfoJson = std::move(doc.toJson(QJsonDocument::JsonFormat::Compact));
}

void Manager::processFinishedHandler(Log::Log log, const int exitCode, const QProcess::ExitStatus exitStatus, const QSharedPointer<QProcess> cmd)
{
    if (exitCode == 0)
    {
        log.result = true;
        KS::Log::Manager::m_logManager->writeLog(log);
        return;
    }
    cmd->program();
    cmd->arguments().join(" ");
    auto errorMsg = cmd->readAll();
    KLOG_ERROR() << "execute cmd: " << cmd->program() << " " << cmd->arguments().join(" ")
                 << ", exitCode: " << exitCode << ", output: " << errorMsg;

    log.result = false;
    log.logMsg += tr(", exitCode %1, error msg: %2").arg(exitCode).arg(QString(errorMsg));
    KS::Log::Manager::m_logManager->writeLog(log);
}

void Manager::hazardDetected(uint type, const QString& alert_msg)
{
    emit m_toolBoxManager->HazardDetected(type, alert_msg);
}

};  // namespace ToolBox
};  // namespace KS
