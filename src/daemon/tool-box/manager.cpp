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
#include <selinux/selinux.h>
#include <src/daemon/account/manager.h>
#include <src/daemon/log/manager.h>
#include <src/daemon/log/message.h>
#include <src/daemon/tool_box_adaptor.h>
#include <QDateTime>
#include "include/ssr-i.h"
#include "src/daemon/common/dbus-helper.h"

#define SHRED_PATH "/usr/bin/shred -f -u"
#define USERDEL_PATH "/usr/sbin/userdel -m"
#define SED_PATH "/usr/bin/sed"
#define DISABLE_SELINUX "-i -re 's/SELINUX=(enforcing|permissive|disabled)/SELINUX=disabled/' /etc/selinux/config"
#define ENABLE_SELINUX "-i -re 's/SELINUX=(enforcing|permissive|disabled)/SELINUX=enforcing/' /etc/selinux/config"

namespace KS
{
namespace ToolBox
{

#pragma message("TODO: 使用 dbus 变量标识当前 selinux 状态")

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
{
    new ToolBoxAdaptor(this);
    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if (!dbusConnection.registerObject(SSR_TOOL_BOX_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "Register ToolBox DBus object error:" << dbusConnection.lastError().message();
    }
}

void Manager::SetAccessControlStatus(bool enable)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
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
        SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, "Failed to set access control status", false);
        return;
    }
    SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, QString("set access control status to ") + (enable ? "enable" : "disable"));
}

QString Manager::GetSecurityContext(const QString& filePath)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    char* context = nullptr;
    if (getfilecon(filePath.toLocal8Bit(), &context) == -1)
    {
        KLOG_ERROR() << "Failed to get " << filePath
                     << "selinux context, error message: " << strerror(errno);
        freecon(context);
        SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, QString("Failed to get %1 selinux context").arg(filePath), false);
        return QString();
    }
    QString rs(context);
    freecon(context);
    SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, QString("Get %1 selinux context").arg(filePath));
    return rs;
}

void Manager::SetSecurityContext(const QString& filePath, const QString& SecurityContext)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (setfilecon(filePath.toLocal8Bit(), SecurityContext.toLocal8Bit()) == -1)
    {
        KLOG_ERROR() << "Failed to set " << filePath
                     << " selinux context: " << SecurityContext
                     << "error message: " << strerror(errno);
        SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, QString("Failed to set %1 selinux context").arg(filePath), false);
        return;
    }
    SSR_LOG(role, Log::Manager::LogType::TOOL_BOX, QString("Set %1 selinux context").arg(filePath));
}

void Manager::ShredFile(const QStringList& filePath)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    Log::Log log = {role, QDateTime::currentDateTime(), Log::Manager::LogType::TOOL_BOX, false, "Shred file"};
    auto cmd = getProcess(log, SHRED_PATH, filePath);
    cmd->startDetached();
}

void Manager::RemoveUser(const QStringList& userNames)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    Log::Log log = {role, QDateTime::currentDateTime(), Log::Manager::LogType::TOOL_BOX, false, "Remove user"};
    for (const auto userName : userNames)
    {
        auto cmd = getProcess(log, USERDEL_PATH, QStringList(userName));
        cmd->start();
    }
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
};  // namespace ToolBox
};  // namespace KS