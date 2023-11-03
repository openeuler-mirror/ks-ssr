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

#include <selinux/selinux.h>
#include <src/daemon/account/manager.h>
#include <src/daemon/log/manager.h>
#include <src/daemon/log/message.h>
#include <src/daemon/security-box/manager.h>
#include <src/daemon/security_box_adaptor.h>

#define SSR_SECURITY_BOX_DBUS_OBJECT_PATH "/com/kylinsec/SSR/SecurityBox"
#define SHRED_PATH "/usr/bin/shred -f -u"
#define USERDEL_PATH "/usr/sbin/userdel -m"
#define SED_PATH "/usr/bin/sed"
#define DISABLE_SELINUX "-i -re 's/SELINUX=(enforcing|permissive|disabled)/SELINUX=disabled/' /etc/selinux/config"
#define ENABLE_SELINUX "-i -re 's/SELINUX=(enforcing|permissive|disabled)/SELINUX=enforcing/' /etc/selinux/config"

namespace KS
{
namespace SecurityBox
{

#pragma message("TODO: 使用 dbus 变量标识当前 selinux 状态")

Manager* Manager::m_securityBoxManager = nullptr;

void Manager::globalInit()
{
    if (m_securityBoxManager == nullptr)
    {
        m_securityBoxManager = new Manager();
    }
}

void Manager::globalDeinit()
{
    delete m_securityBoxManager;
}

Manager::Manager()
{
    new SecurityBoxAdaptor(this);
    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if (!dbusConnection.registerObject(SSR_SECURITY_BOX_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "Register SecurityBox DBus object error:" << dbusConnection.lastError().message();
    }
}

#pragma message("TODO: 不应该由接口来传入 role, 应该获取当前 callerPid 从 account 中取")
void Manager::SetAccessControlStatus(bool enable, int role)
{
    if (static_cast<Account::Manager::Role>(role) != Account::Manager::Role::SECADMIN)
    {
        KLOG_WARNING() << "Only secadmin can modify selinux";
        return;
    }
    QStringList arg{};
    if (enable)
    {
        arg = QString(ENABLE_SELINUX).split(" ");
    }
    else
    {
        arg = QString(DISABLE_SELINUX).split(" ");
    }
    auto cmd = getProcess(SED_PATH, arg);
    cmd->startDetached();
}

QString Manager::GetSecurityContext(const QString& filePath)
{
    char* context = nullptr;
    if (getfilecon(filePath.toLocal8Bit(), &context) == -1)
    {
        KLOG_ERROR() << "Failed to get " << filePath
                     << "selinux context, error message: " << strerror(errno);
        freecon(context);
        return QString();
    }
    QString rs(context);
    freecon(context);
    return rs;
}

void Manager::SetSecurityContext(const QString& filePath, const QString& SecurityContext)
{
    if (setfilecon(filePath.toLocal8Bit(), SecurityContext.toLocal8Bit()) == -1)
    {
        KLOG_ERROR() << "Failed to set " << filePath
                     << " selinux context: " << SecurityContext
                     << "error message: " << strerror(errno);
    }
}

void Manager::SherdFile(const QStringList& filePath)
{
    auto cmd = getProcess(SHRED_PATH, filePath);
    cmd->startDetached();
}

void Manager::RemoveUser(const QStringList& userNames)
{
    std::for_each(userNames.begin(), userNames.end(), [](const QString& userName) {
        auto cmd = getProcess(USERDEL_PATH, QStringList(userName));
        cmd->start();
    });
}

void Manager::processFinishedHandler(const int exitCode, const QProcess::ExitStatus exitStatus, const QSharedPointer<QProcess> cmd)
{
    QString logMsg{};
    QTextStream logMsgStream{&logMsg};
    if (exitCode != 0)
    {
        KLOG_WARNING() << "Failed to execute cmd";
    }
    cmd->program();
    cmd->arguments().join(" ");
    logMsgStream << "execute cmd: " << cmd->program() << " " << cmd->arguments().join(" ")
                 << ", exitCode: " << exitCode << ", output: " << cmd->readAll();
    logMsgStream.flush();
    Log::Manager::writeLog(Log::Message{Log::Message::LogType::SECURITY_BOX, logMsg}.serialize());
}
};  // namespace SecurityBox
};  // namespace KS