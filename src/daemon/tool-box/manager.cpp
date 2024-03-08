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
#include <unistd.h>
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
#include "src/daemon/common/polkit-proxy.h"
#include "src/daemon/tool-box/manager.h"
#include "src/daemon/tool-box/realtime-alert.h"

#define RBAPOL_PATH "/usr/bin/rbapol"
#define RBAUSER_PATH "/usr/bin/rbauser"

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

// 在文件粉碎功能中可能会需要删除非常大量的文件，考虑到 ARG_MAX 等限制，所以限制一次最多删除一百个文件
#define FILE_LIST_LIMIT 100

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
    updateAccountInfo();
    connect(m_userNameWatcher, &QFileSystemWatcher::fileChanged, [this](const QString&)
            {
                // linux 创建和删除用户时， 可能会存在 /etc/passwd 文件删除又创建的情况
                this->m_userNameWatcher->removePath(PASSWD_FILE);
                if (QFile::exists(PASSWD_FILE))
                {
                    this->m_userNameWatcher->addPath(PASSWD_FILE);
                }
                this->updateAccountInfo();
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

CHECK_AUTH_WITH_1ARGS(Manager, SetAccessControlStatus, setAccessControlStatus, SSR_PERMISSION_AUTHENTICATION, bool);
CHECK_AUTH_WITH_1ARGS(Manager, RemoveUser, removeUser, SSR_PERMISSION_AUTHENTICATION, const QStringList&);
CHECK_AUTH_WITH_1ARGS(Manager, ShredFile, shredFile, SSR_PERMISSION_AUTHENTICATION, const QStringList&);
CHECK_AUTH_WITH_2ARGS(Manager, SetFileMLSLabel, setFileMLSLabel, SSR_PERMISSION_AUTHENTICATION, const QString&, const QString&);
CHECK_AUTH_WITH_2ARGS(Manager, SetFileKICLabel, setFileKICLabel, SSR_PERMISSION_AUTHENTICATION, const QString&, const QString&);
CHECK_AUTH_WITH_2ARGS(Manager, SetUserMLSLabel, setUserMLSLabel, SSR_PERMISSION_AUTHENTICATION, const QString&, const QString&);

void Manager::setAccessControlStatus(const QDBusMessage& message, bool enable)
{
    auto calledUniqueName = message.service();
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set access control status, permission denied"),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, message);
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
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_ACCESS_CONTROL, message);
    }
    DBUS_REPLY(message);
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("set access control status to %1").arg(enable ? "enable" : "disable"),
                    calledUniqueName);
}

QString Manager::GetFileMLSLabel(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_COMMON_INVALID_ARGS, this->message());
    }
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    auto userName = Account::Manager::m_accountManager->getUserName(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }

    Log::Log log = {userName, role, QDateTime::currentDateTime(),
                    Log::Manager::LogType::TOOL_BOX, false, tr("Get files %1 mls label").arg(filePath)};
    QString output;
    if (!getFileSeLabels(filePath, output, SeLabelType::MLS))
    {
        KLOG_ERROR() << tr("Failed to get files %1 mls label, error msg %2").arg(filePath).arg(output);
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to get files %1 mls label, error msg %2")
                          .arg(filePath)
                          .arg(output),
                      calledUniqueName);
        return "";
    }
    return QString(output).split(": ")[1].trimmed();
}

QString Manager::GetFileKICLabel(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_COMMON_INVALID_ARGS, this->message());
    }
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    auto userName = Account::Manager::m_accountManager->getUserName(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }

    Log::Log log = {userName, role, QDateTime::currentDateTime(),
                    Log::Manager::LogType::TOOL_BOX, false, tr("Get files %1 kic label").arg(filePath)};
    QString output;
    if (!getFileSeLabels(filePath, output, SeLabelType::KIC))
    {
        KLOG_ERROR() << tr("Failed to get files %1 kic label, error msg %2").arg(filePath).arg(output);
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to get files %1 kic label, error msg %2")
                          .arg(filePath)
                          .arg(output),
                      calledUniqueName);
        return "";
    }
    return QString(output).split(": ")[1].trimmed();
}

void Manager::setFileMLSLabel(const QDBusMessage& message, const QString& filePath, const QString& SecurityContext)
{
    if (filePath.isEmpty() || SecurityContext.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message);
    }
    auto calledUniqueName = message.service();
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set mls label, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, message);
    }
    QString output;

    if (!setFileSeLabels(filePath, SecurityContext, output, SeLabelType::MLS))
    {
        KLOG_ERROR() << "Failed to set " << filePath
                     << " security label: " << SecurityContext
                     << "error message: " << output;
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set %1 mls label, error msg: %2").arg(filePath).arg(output),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_MLS_CONTEXT, message);
        return;
    }
    DBUS_REPLY(message);
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Set %1 mls label to: %2").arg(filePath).arg(SecurityContext),
                    calledUniqueName);
    emit FileSignListChanged();
}

void Manager::setFileKICLabel(const QDBusMessage& message, const QString& filePath, const QString& SecurityContext)
{
    if (filePath.isEmpty() || SecurityContext.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message);
    }
    auto calledUniqueName = message.service();
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set kic label, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, message);
    }

    QString output;
    if (!setFileSeLabels(filePath, SecurityContext, output, SeLabelType::KIC))
    {
        KLOG_ERROR() << "Failed to set " << filePath
                     << " security label: " << SecurityContext
                     << "error message: " << output;
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set %1 kic label, error msg: %2").arg(filePath).arg(output),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_KIC_CONTEXT, message);
    }
    DBUS_REPLY(message);
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Set %1 kic label to: %2").arg(filePath).arg(SecurityContext),
                    calledUniqueName);
    emit FileSignListChanged();
}

QString Manager::GetUserMLSLabel(const QString& userName)
{
    if (userName.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_COMMON_INVALID_ARGS, this->message());
    }
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    auto _userName = Account::Manager::m_accountManager->getUserName(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }

    Log::Log log = {_userName, role, QDateTime::currentDateTime(),
                    Log::Manager::LogType::TOOL_BOX, false, tr("Get files %1 mls label").arg(userName)};
    QRegularExpression regex("(s[\\d+].*)");
    QString output;
    if (!getUserSeLabels(userName, output))
    {
        KLOG_ERROR() << tr("Failed to get user %1 mls label, error msg %2").arg(userName).arg(output);
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to get user %1 mls label, error msg %2")
                          .arg(userName)
                          .arg(output),
                      calledUniqueName);
        return "";
    }
    auto match = regex.match(output);
    return match.captured(1).trimmed();
}

void Manager::setUserMLSLabel(const QDBusMessage& message, const QString& userName, const QString& SecurityContext)
{
    if (userName.isEmpty() || SecurityContext.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message);
    }
    auto calledUniqueName = message.service();
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set user mls label, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, message);
    }

    QString output;
    if (!setUserSeLabels(userName, SecurityContext, output))
    {
        KLOG_ERROR() << "Failed to set " << userName
                     << " mls label: " << SecurityContext
                     << "error message: " << output;
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set %1 mls label, error msg: %2").arg(userName).arg(output),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_MLS_CONTEXT, message);
    }
    DBUS_REPLY(message);
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Set %1 user mls label to: %2").arg(userName).arg(SecurityContext),
                    calledUniqueName);
    emit FileSignListChanged();
}

void Manager::shredFile(const QDBusMessage& message, const QStringList& targetPath)
{
    if (targetPath.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message);
    }
    auto calledUniqueName = message.service();
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    auto userName = Account::Manager::m_accountManager->getUserName(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to shred file, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, message);
    }
    // 传进来的链表中可能有文件夹， 所以需要遍历所有链表， 如果其中有文件，则把文件夹下所有的内容追加至遍历内容尾
    // 类似于二叉树的中序遍历
    QStringList needShred{targetPath};
    QStringList failedToShred{};
    QStringList dir{};
    uint index = 0;
    uint size = needShred.size();
    while (index < size)
    {
        if (index >= FILE_LIST_LIMIT)
        {
            auto firstIt = needShred.begin();
            // index 当前指向的节点不一定是文件， 所以需要减 - 1，但是由于 cpp 容器的迭代器操作是左闭右开的，所以 + 1，抵消
            // Qt 5.11 不支持迭代器初始化
            QStringList Shred{needShred.mid(0, index)};
            if (!shred(Shred))
            {
                // 失败的情况下需要确定哪些文件未被删除
                failedToShred.append(isPathsExist(Shred));
            }
            needShred.erase(firstIt, firstIt + index);
            index = 0;
        }
        QString path = needShred[index];
        QFileInfo fileInfo{path};
        // 如果文件不存在不需要特殊处理， 在校验粉碎是否成功时会将其判断为成功粉碎
        if (!fileInfo.exists() || fileInfo.isFile())
        {
            index++;
            continue;
        }
        // 如果当前下标指向一个文件夹时，不需要将 index 向前推进， 因为当前指向的元素将会被移除， index 自然指向下一个元素
        auto entryList = QDir{path}.entryList(
            QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden);
        // 拼成绝对路径在加入链表
        for (auto it = entryList.begin(); it != entryList.end(); ++it)
        {
            *it = path + '/' + *it;
        }
        needShred.append(entryList);
        // 将文件加单独拿出来， shred 不能删除文件夹， 所以需要单独删除文件夹。
        dir.append(needShred.takeAt(index));
        size = needShred.size();
    }
    if (!shred(needShred))
    {
        // 失败的情况下需要确定哪些文件未被删除
        failedToShred.append(isPathsExist(needShred));
    }

    for (const auto& dirPath : dir)
    {
        if (!targetPath.contains(dirPath))
        {
            continue;
        }
        QDir dir{dirPath};
        dir.removeRecursively();
    }
    QStringList shreddedFile{};
    for (const auto& path : targetPath)
    {
        // 检查本次删除成功删除了哪些文件， 如果有文件被成功粉碎且其也在数据库中， 则将其从数据库中删除
        if (!QFileInfo{path}.exists())
        {
            shreddedFile.append(path);
        }
    }

    if (!removeFileFromFileShred(shreddedFile))
    {
        KLOG_WARNING() << "Failed to remove file from FileShredList, files: " << shreddedFile;
    }

    if (!failedToShred.isEmpty())
    {
        DBUS_ERROR_REPLY(SSRErrorCode::ERROR_TOOL_BOX_FAILED_SHRED_FILES, message);
        KLOG_ERROR() << "Failed to shred file: " << failedToShred;
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to shred file: %1").arg(failedToShred.join(',')),
                      calledUniqueName);
    }

    if (!shreddedFile.isEmpty())
    {
        SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                        tr("Shred files %1").arg(shreddedFile.join(',')),
                        calledUniqueName);
        DBUS_REPLY(message);
    }
}

bool Manager::shred(const QStringList& targetPaths)
{
    auto cmd = getProcess(SHRED_PATH, QStringList{SHRED_ARG_1, SHRED_ARG_2} << targetPaths);
    cmd->start();
    cmd->waitForFinished();
    sync();
    return !cmd->exitCode();
}

void Manager::removeUser(const QDBusMessage& message, const QStringList& userNames)
{
    if (userNames.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message);
    }
    auto calledUniqueName = message.service();
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    auto userName = Account::Manager::m_accountManager->getUserName(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to remove user, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, message);
    }

    QStringList failedToRemove{};
    QStringList removed{};
    for (const auto& userName : userNames)
    {
        auto cmd = getProcess(USERDEL_PATH, QStringList(userName));
        cmd->start();
        cmd->waitForFinished();
        if (cmd->exitCode())
        {
            QString output = cmd->readAllStandardError() + cmd->readAllStandardOutput();
            KLOG_ERROR() << "Failed to remove user " << userName
                         << "error message: " << output;
            SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                          tr("Failed to remove user %1, error msg: %2").arg(userName).arg(output),
                          calledUniqueName);
            failedToRemove.append(userName);
        }
        else
        {
            removed.append(userName);
        }
    }
    QDBusMessage replyMessage;
    if (!failedToRemove.isEmpty())
    {
        DBUS_ERROR_REPLY(SSRErrorCode::ERROR_TOOL_BOX_FAILED_REMOVE_USERS, message);
    }

    if (removed.isEmpty())
    {
        return;
    }
    DBUS_REPLY(message);
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX, tr("Remove users: %1").arg(removed.join(',')), calledUniqueName);
    QStringList userSpace{};
    for (const auto& removedUser : removed)
    {
        for (const auto& userInfo : m_osUserInfo)
        {
            if (removedUser != userInfo.name || userInfo.dir.isEmpty())
            {
                continue;
            }
            // 获取删除
            userSpace.append(userInfo.dir);
            break;
        }
    }
    std::thread shredUserSpace{&Manager::shredFile, this, message, userSpace};
    shredUserSpace.detach();
}

bool Manager::GetAccessStatus()
{
    return static_cast<bool>(is_selinux_enabled());
}

QStringList Manager::GetObjListFromSecuritySign()
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

void Manager::AddObjToSecuritySign(const QStringList& objList)
{
    if (objList.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, this->message());
    }
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to add files to SignFile list, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    // 如果传入的是用户名则校验用户是否存在
    // 筛选出用户存在的列表
    QStringList validList{};
    QStringList invalidList{};
    for (const auto& obj : objList)
    {
        CONTINUE_IF_TRUE(obj.isEmpty());
        if (!obj.startsWith('/') && getpwnam(obj.toLocal8Bit()) == nullptr)
        {
            invalidList.append(obj);
            continue;
        }
        validList.append(obj);
    }

    if (!invalidList.isEmpty())
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to add user to object list, unknown user %1").arg(invalidList.join(' ')),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TOOL_BOX_FAILED_ADD_OBJ_TO_SECURITY_SIGN, this->message());
    }

    if (validList.isEmpty())
    {
        return;
    }
    constexpr const char* insertFileToFileSign = "insert OR IGNORE into " FILE_SIGN_TABLE
                                                 " values ('%1');";
    QString insertFileToFileSignDBCmd{insertFileToFileSign};
    if (!m_db->exec(insertFileToFileSignDBCmd.arg(validList.join("'), ('"))))
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to add object to object list, database error"),
                      calledUniqueName)
        KLOG_ERROR() << "Failed to add object: " << validList;
    }
    emit FileSignListChanged();
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Add obj to object list: %1").arg(validList.join(' ')),
                    calledUniqueName);
}

void Manager::RemoveObjFromSecuritySign(const QStringList& objList)
{
    if (objList.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, this->message());
    }
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to remove obj from obj list, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    constexpr const char* removeFileFromFileSign = "delete from " FILE_SIGN_TABLE
                                                   " where " FILE_SIGN_COLUMN1
                                                   " in ('%1');";
    QString removeFileToFileSignDBCmd{removeFileFromFileSign};
    if (!m_db->exec(removeFileToFileSignDBCmd.arg(objList.join("', '"))))
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to remove file from object list, database error"),
                      calledUniqueName)
        KLOG_ERROR() << "Failed to remove object: " << objList;
    }
    emit FileSignListChanged();
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Remove file from object list: %1").arg(objList.join(' ')),
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

void Manager::AddFileToFileShred(const QStringList& fileList)
{
    if (fileList.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, this->message());
    }
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
    if (!m_db->exec(insertFileToFileShredDBCmd.arg(fileList.join("'), ('"))))
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to add files to ShredFile list, database error"),
                      calledUniqueName)
        KLOG_ERROR() << "Failed to add files: " << fileList;
    }
    emit FileShredListChanged();
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Add file to ShredFile list: %1").arg(fileList.join(' ')),
                    calledUniqueName);
}

void Manager::RemoveFileFromFileShred(const QStringList& fileList)
{
    if (fileList.isEmpty())
    {
        KLOG_WARNING() << "invalid parameter, return";
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, this->message());
    }
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to remove file from ShredFile list, permission denied"),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }

    if (!removeFileFromFileShred(fileList))
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to remove file from ShredFile list, database error"),
                      calledUniqueName);
        KLOG_ERROR() << "Failed to remove files: " << fileList;
    }

    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Remove file from ShredFile list: %1").arg(fileList.join(' ')),
                    calledUniqueName);
}

bool Manager::removeFileFromFileShred(const QStringList& fileList)
{
    QStringList valid{};
    auto existed = GetFileListFromFileShred();
    for (const auto& file : fileList)
    {
        if (existed.contains(file))
        {
            valid.append(file);
        }
    }
    if (valid.isEmpty())
    {
        return true;
    }
    constexpr const char* removeFileFromFileShred = "delete from " FILE_SHRED_TABLE
                                                    " where " FILE_SHRED_COLUMN1
                                                    " in ('%1');";
    QString removeFileToFileShredDBCmd{removeFileFromFileShred};
    if (!m_db->exec(removeFileToFileShredDBCmd.arg(fileList.join("', '"))))
    {
        return false;
    }
    emit FileShredListChanged();
    return true;
}

QString Manager::GetAllUsers()
{
    updateAccountInfo();
    QReadLocker locker(m_osUserNameMutex);
    QJsonArray arr;
    QStringList managerUserList;
    const QString managerGroup("wheel");
    for (const auto& groupInfo : m_osGroupInfo)
    {
        CONTINUE_IF_TRUE(groupInfo.name != managerGroup);
        for (const auto& member : groupInfo.mem)
        {
            QJsonObject obj;
            managerUserList.append(member);
            obj.insert("name", member);
            obj.insert("type", OsUserType::USER_TYPE_MANAGER);
            arr.append(obj);
        }
        break;
    }
    for (const auto& userInfo : m_osUserInfo)
    {
        CONTINUE_IF_TRUE(managerUserList.contains(userInfo.name));
        CONTINUE_IF_TRUE(userInfo.uid < 1000);
        // linux系统中存在一个nobody的系统用户，权限很低，一般用于运行服务、访问受限资源、处理匿名访问等，这里需要将这个用户过滤掉
        CONTINUE_IF_TRUE(userInfo.name == "nobody");
        QJsonObject obj;
        obj.insert("name", userInfo.name);
        obj.insert("type", OsUserType::USER_TYPE_NORMAL);
        arr.append(obj);
    }
    QJsonDocument doc(arr);
    return doc.toJson(QJsonDocument::JsonFormat::Compact);
}

void Manager::updateAccountInfo(const QString&)
{
    QWriteLocker locker(m_osUserNameMutex);
    m_osGroupInfo.clear();
    m_osUserInfo.clear();
    struct group* groupInfo;
    setgrent();
    // KS::ToolBox::Group
    while ((groupInfo = getgrent()) != nullptr)
    {
        QStringList memberList{};
        char** members = groupInfo->gr_mem;
        while (*members != nullptr)
        {
            memberList.append(*members++);
        }
        m_osGroupInfo.append({groupInfo->gr_gid, groupInfo->gr_name, groupInfo->gr_passwd, memberList});
    }
    endgrent();

    struct passwd* pw;
    setpwent();  // 重置密码文件的读取位置到开头
    while ((pw = getpwent()) != nullptr)
    {
        m_osUserInfo.append({pw->pw_name, pw->pw_passwd, pw->pw_uid, pw->pw_gid,
                             pw->pw_gecos, pw->pw_dir, pw->pw_shell});
    }
    // KLOG_DEBUG() << "m_osUserInfo: " << m_osUserInfo;
    endpwent();  // 关闭密码文件
}

void Manager::processFinishedHandler(Log::Log log, const int exitCode, const QProcess::ExitStatus exitStatus, const QSharedPointer<QProcess> cmd)
{
    if (exitCode == 0)
    {
        log.result = true;
        KS::Log::Manager::m_logManager->writeLog(log);
        return;
    }
    auto errorMsg = cmd->readAllStandardOutput() + cmd->readAllStandardError();
    KLOG_ERROR() << "execute cmd: " << cmd->program() << " " << cmd->arguments().join(" ")
                 << ", exitCode: " << exitCode << ", output: " << errorMsg;

    log.result = false;
    log.logMsg += tr(" failed, exitCode %1, error msg: %2").arg(exitCode).arg(QString(errorMsg));
    KS::Log::Manager::m_logManager->writeLog(log);
}

void Manager::hazardDetected(uint type, const QString& alertMsg)
{
    emit m_toolBoxManager->HazardDetected(type, alertMsg);
}

bool Manager::setFileSeLabels(const QString& filePath, const QString& seLabel, QString& output, const SeLabelType seLabelType)
{
    output.clear();
    QProcess cmd{};
    cmd.setProgram(RBAPOL_PATH);
    switch (seLabelType)
    {
    case Manager::SeLabelType::MLS:
        cmd.setArguments({"-s", "-k", QString("mls/%1").arg(seLabel), "-f", filePath});
        break;
    case Manager::SeLabelType::KIC:
        cmd.setArguments({"-s", "-k", QString("kic/%1").arg(seLabel), "-f", filePath});
        break;
    default:
        KLOG_ERROR() << "Failed to set file selabel, unknown selabel type: " << static_cast<int>(seLabelType);
        return false;
        break;
    }
    cmd.start();
    cmd.waitForFinished();
    cmd.waitForReadyRead();
    output = cmd.readAllStandardError() + cmd.readAllStandardOutput();
    return !cmd.exitCode();
}

bool Manager::getFileSeLabels(const QString& filePath, QString& output, const SeLabelType seLabelType)
{
    output.clear();
    QProcess cmd{};
    cmd.setProgram(RBAPOL_PATH);
    switch (seLabelType)
    {
    case Manager::SeLabelType::MLS:
        cmd.setArguments({"-g", "-k", QString("mls"), "-f", filePath});
        break;
    case Manager::SeLabelType::KIC:
        cmd.setArguments({"-g", "-k", QString("kic"), "-f", filePath});
        break;
    default:
        KLOG_ERROR() << "Failed to set file selabel, unknown selabel type: " << static_cast<int>(seLabelType);
        return false;
        break;
    }
    cmd.start();
    cmd.waitForFinished();
    cmd.waitForReadyRead();
    output = cmd.readAllStandardError() + cmd.readAllStandardOutput();
    return !cmd.exitCode();
}

bool Manager::setUserSeLabels(const QString& userName, const QString& seLabel, QString& output)
{
    output.clear();
    QProcess setUserSeLabel{};
    setUserSeLabel.setProgram(RBAUSER_PATH);
    setUserSeLabel.setArguments({"-s", "-M", seLabel, userName});
    setUserSeLabel.start();
    setUserSeLabel.waitForFinished();
    setUserSeLabel.waitForReadyRead();
    output = setUserSeLabel.readAllStandardError() + setUserSeLabel.readAllStandardOutput();
    return !setUserSeLabel.exitCode();
}

bool Manager::getUserSeLabels(const QString& userName, QString& output)
{
    output.clear();
    // 系统用户需要绑定安全角色才能 获取/设置 安全上下文
    QProcess bindUser{};
    bindUser.setProgram(RBAUSER_PATH);
    bindUser.setArguments({"-n", "-M", "s0", "-N", "user_u", userName});
    bindUser.start();
    // 绑定用户至安全角色是否成功不影响流程
    bindUser.waitForFinished();
    if (!!bindUser.exitCode())
    {
        KLOG_DEBUG() << "Failed to bind " << userName << " to rbarole: user_u"
                    << "output: " << bindUser.readAll();
    }

    QProcess getUserSeLabel{};
    getUserSeLabel.setProgram(RBAUSER_PATH);
    getUserSeLabel.setArguments({"-q", userName});
    getUserSeLabel.start();
    getUserSeLabel.waitForFinished();
    getUserSeLabel.waitForReadyRead();
    output = getUserSeLabel.readAllStandardError() + getUserSeLabel.readAllStandardOutput();
    return !getUserSeLabel.exitCode();
}

QStringList Manager::isPathsExist(const QStringList& paths)
{
    QStringList existPaths{};
    for (const auto& path : paths)
    {
        if (QFileInfo{path}.exists())
        {
            existPaths.append(path);
        }
    }
    return existPaths;
}

};  // namespace ToolBox
};  // namespace KS
