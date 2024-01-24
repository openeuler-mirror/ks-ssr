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
      m_realTimeAlert(new RealTimeAlert())
{
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
    arg.append(enable ? R"({s#SELINUX=(enforcing|permissive|disabled)#SELINUX=enforcing#})" : R"({s/SELINUX=(enforcing|permissive|disabled)/SELINUX=disabled/})");
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
    Log::Log log = {userName, role, QDateTime::currentDateTime(), Log::Manager::LogType::TOOL_BOX, false, "Shred file"};
    // TODO : 粉碎文件夹不生效
    auto cmd = getProcess(log, SHRED_PATH, QStringList{SHRED_ARG_1, SHRED_ARG_2} << filePath);
    cmd->startDetached();
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
    Log::Log log = {userName, role, QDateTime::currentDateTime(), Log::Manager::LogType::TOOL_BOX, false, "Remove user"};
    for (const auto& userName : userNames)
    {
        auto cmd = getProcess(log, USERDEL_PATH, QStringList{USERDEL_ARG_1} << userName);
        cmd->startDetached();
    }
}

bool Manager::GetAccessStatus()
{
    return static_cast<bool>(is_selinux_enabled());
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

void Manager::processFinishedHandler(Log::Log& log, const int exitCode, const QProcess::ExitStatus exitStatus, const QSharedPointer<QProcess> cmd)
{
    if (exitCode == 0)
    {
        log.result = true;
        KS::Log::Manager::m_logManager->writeLog(log);
        return;
    }
    cmd->program();
    cmd->arguments().join(" ");
    KLOG_ERROR() << "execute cmd: " << cmd->program() << " " << cmd->arguments().join(" ")
                 << ", exitCode: " << exitCode << ", output: " << cmd->readAll();

    log.result = false;
    KS::Log::Manager::m_logManager->writeLog(log);
}

void Manager::hazardDetected(uint type, const QString& alert_msg)
{
    emit m_toolBoxManager->HazardDetected(type, alert_msg);
}

};  // namespace ToolBox
};  // namespace KS