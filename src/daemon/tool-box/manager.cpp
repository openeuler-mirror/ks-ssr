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

CHECK_AUTH_WITH_1ARGS(Manager, SetAccessControlStatus, setAccessControlStatus, SSR_PERMISSION_AUTHENTICATION, bool);
CHECK_AUTH_WITH_1ARGS(Manager, RemoveUser, removeUser, SSR_PERMISSION_AUTHENTICATION, const QStringList&);
CHECK_AUTH_WITH_2ARGS(Manager, SetFileMLSLabel, setFileMLSLabel, SSR_PERMISSION_AUTHENTICATION, const QString&, const QString&);
CHECK_AUTH_WITH_2ARGS(Manager, SetFileKICLabel, setFileKICLabel, SSR_PERMISSION_AUTHENTICATION, const QString&, const QString&);
CHECK_AUTH_WITH_2ARGS(Manager, SetUserMLSLabel, setUserMLSLabel, SSR_PERMISSION_AUTHENTICATION, const QString&, const QString&);

void Manager::setAccessControlStatus(const QDBusMessage& message, bool enable)
{
    SCOPE_EXIT(
        {
            QDBusConnection::systemBus().send(message.createReply());
        });
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
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("set access control status to %1").arg(enable ? "enable" : "disable"),
                    calledUniqueName);
}

QString Manager::GetFileMLSLabel(const QString& filePath)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    auto userName = Account::Manager::m_accountManager->getUserName(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to get mls label, permission denied"),
                      calledUniqueName);
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
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    auto userName = Account::Manager::m_accountManager->getUserName(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to get kic label, permission denied"),
                      calledUniqueName);
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
    SCOPE_EXIT(
        {
            QDBusConnection::systemBus().send(message.createReply());
        });
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
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_SECURITY_CONTEXT, message);
        return;
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Set %1 mls label to: %2").arg(filePath).arg(SecurityContext),
                    calledUniqueName);
    emit FileSignListChanged();
}

void Manager::setFileKICLabel(const QDBusMessage& message, const QString& filePath, const QString& SecurityContext)
{
    SCOPE_EXIT(
        {
            QDBusConnection::systemBus().send(message.createReply());
        });
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
                      tr("Failed to set %1 mls label, error msg: %2").arg(filePath).arg(output),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_SECURITY_CONTEXT, message);
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Set %1 kic label to: %2").arg(filePath).arg(SecurityContext),
                    calledUniqueName);
    emit FileSignListChanged();
}

QString Manager::GetUserMLSLabel(const QString& userName)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    auto _userName = Account::Manager::m_accountManager->getUserName(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to get kic label, permission denied"),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QString(), SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }

    Log::Log log = {_userName, role, QDateTime::currentDateTime(),
                    Log::Manager::LogType::TOOL_BOX, false, tr("Get files %1 kic label").arg(userName)};
    QRegularExpression regex("(s[\\d+])");
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
    return match.captured(1);
}

void Manager::setUserMLSLabel(const QDBusMessage& message, const QString& userName, const QString& SecurityContext)
{
    SCOPE_EXIT(
        {
            QDBusConnection::systemBus().send(message.createReply());
        });
    auto calledUniqueName = message.service();
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set user mls label, permission denied"),
                      calledUniqueName)
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, message);
    }
    if (userName.isEmpty() || SecurityContext.isEmpty())
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set user mls label, invalid parameter"),
                      calledUniqueName)
        auto replyMessage = message
                                .createErrorReply(QDBusError::Failed, tr("Failed to set user mls label, invalid parameter"));
        QDBusConnection::systemBus().send(replyMessage);
        return;
    }

    QString output;
    if (!setUserSeLabels(userName, SecurityContext, output))
    {
        KLOG_ERROR() << "Failed to set " << userName
                     << " security label: " << SecurityContext
                     << "error message: " << output;
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to set %1 mls label, error msg: %2").arg(userName).arg(output),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_SECURITY_CONTEXT, message);
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Set %1 user mls label to: %2").arg(userName).arg(SecurityContext),
                    calledUniqueName);
    emit FileSignListChanged();
}

void Manager::ShredFile(const QStringList& targetPath)
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
                    Log::Manager::LogType::TOOL_BOX, false, tr("Shred files %1").arg(targetPath.join(' '))};
    // 传进来的链表中可能有文件夹， 所以需要遍历所有链表， 如果其中有文件，则把文件夹下所有的内容追加至遍历内容尾
    // 类似于二叉树的中序遍历
    QStringList allFileList{targetPath};
    QStringList dirList{};
    uint index = 0;
    uint size = allFileList.size();
    while (index < size)
    {
        QString path = allFileList[index];
        QFileInfo fileInfo{path};
        if (fileInfo.isFile())
        {
            index++;
            continue;
        }
        // 如果当前下标指向一个文件夹时，不需要将 index 向前推进， 因为当前指向的元素将会被移除， index 自然指向下一个元素
        QDir dir{path};
        auto entryList = dir.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden);
        // 拼成绝对路径在加入链表
        for (auto it = entryList.begin(); it != entryList.end(); ++it)
        {
            *it = path + '/' + *it;
        }
        allFileList.append(entryList);
        // 将文件加单独拿出来， shred 不能删除文件夹， 所以需要单独删除文件夹。
        dirList.append(allFileList.takeAt(index));
        size = allFileList.size();
    }
    auto cmd = getProcess(log /*unused*/, SHRED_PATH, QStringList{SHRED_ARG_1, SHRED_ARG_2} << allFileList);
    cmd->start();
    cmd->waitForFinished();
    if (!!cmd->exitCode())
    {
        auto replyMessage = this->message().createErrorReply(
            QDBusError::Failed,
            tr("Failed to Shred files: %1, see log for more details").arg(targetPath.join(' ')));
        QDBusConnection::systemBus().send(replyMessage);
        return;
    }
    for (const auto& path : dirList)
    {
        if (!targetPath.contains(path))
        {
            continue;
        }
        QDir dir{path};
        dir.removeRecursively();
    }
    RemoveFileFromFileShred(targetPath);
}

void Manager::removeUser(const QDBusMessage& message, const QStringList& userNames)
{
    SCOPE_EXIT(
        {
            QDBusConnection::systemBus().send(message.createReply());
        });
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
        auto replyMessage = this->message().createErrorReply(QDBusError::Failed,
                                                             tr("Failed to add user to object list, unknown user %1").arg(invalidList.join(' ')));
        QDBusConnection::systemBus().send(replyMessage);
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
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    auto role = Account::Manager::m_accountManager->getRole(calledUniqueName);
    if (role != KS::Account::Manager::AccountRole::secadm)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to remove file from ShredFile list, permission denied"),
                      calledUniqueName);
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, this->message());
    }
    constexpr const char* removeFileFromFileShred = "delete from " FILE_SHRED_TABLE
                                                    " where " FILE_SHRED_COLUMN1
                                                    " in ('%1');";
    QString removeFileToFileShredDBCmd{removeFileFromFileShred};
    if (!m_db->exec(removeFileToFileShredDBCmd.arg(fileList.join("', '"))))
    {
        SSR_LOG_ERROR(Log::Manager::LogType::TOOL_BOX,
                      tr("Failed to remove file from ShredFile list, database error"),
                      calledUniqueName);
        KLOG_ERROR() << "Failed to remove files: " << fileList;
    }
    emit FileShredListChanged();
    SSR_LOG_SUCCESS(Log::Manager::LogType::TOOL_BOX,
                    tr("Remove file from ShredFile list: %1").arg(fileList.join(' ')),
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
        CONTINUE_IF_TRUE(groupInfo->gr_name != managerGroup);
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
        CONTINUE_IF_TRUE(managerUserList.contains(pw->pw_name));
        CONTINUE_IF_TRUE(pw->pw_uid < 1000);
        // linux系统中存在一个nobody的系统用户，权限很低，一般用于运行服务、访问受限资源、处理匿名访问等，这里需要将这个用户过滤掉
        CONTINUE_IF_TRUE(QString::fromStdString(pw->pw_name) == "nobody");
        QJsonObject obj;
        obj.insert("name", pw->pw_name);
        obj.insert("type", OsUserType::USER_TYPE_NORMAL);
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
    auto errorMsg = cmd->readAll();
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
        KLOG_INFO() << "Failed to bind " << userName << " to rbarole: user_u"
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

};  // namespace ToolBox
};  // namespace KS
