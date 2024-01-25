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

#pragma once

#include <QDBusContext>
#include <QProcess>
#include <QSharedPointer>
#include "src/daemon/log/manager.h"

class QFileSystemWatcher;
class QReadWriteLock;
namespace KS
{
class Database;
}

namespace KS
{
namespace ToolBox
{

class RealTimeAlert;

class Manager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    static void globalInit();
    static void globalDeinit();
    /**
     * @brief 设置访问控制状态的函数
     * @param enable 开启或关闭访问控制
     * @param role 当前角色，必须要一定权限的角色才可以设置访问控制状态
     */
    void SetAccessControlStatus(bool enable);

    /**
     * @brief 返回指定文件的安全上下文
     * @param filePath 文件路径
     * @return 安全上下文内容
     */
    QString GetSecurityContext(const QString& filePath);

    /**
     * @brief 设置安全上下文
     * @param filePath 文件路径
     * @param SecurityContext 需要设置的安全上下文
     */
    void SetSecurityContext(const QString& filePath, const QString& SecurityContext);

    /**
     * @brief 调用 shred 来彻底删除文件的函数
     * @param filePath 需要删除的文件的路径
     */
    void ShredFile(const QStringList& filePath);

    /**
     * @brief 删除用户及其相关数据
     * @param userName 要删除的用户名
     */
    void RemoveUser(const QStringList& userNames);

    bool GetAccessStatus();

    QString GetAllUsers();

    QStringList GetFileListFromFileSign();

    void AddFileToFileSign(const QStringList& file_list);

    void RemoveFileFromFileSign(const QStringList& file_list);

    QStringList GetFileListFromFileShred();

    void AddFileToFileShred(const QStringList& file_list);

    void RemoveFileFromFileShred(const QStringList& file_list);

    static void hazardDetected(uint type, const QString& alert_msg);

Q_SIGNALS:  // SIGNALS
    void FileShredListChanged();
    void FileSignListChanged();
    void UserChanged();
    void HazardDetected(uint type, const QString& alert_msg);

private:
    Manager();
    virtual ~Manager() = default;
    void initDatabase();
    static void processFinishedHandler(Log::Log log, const int exitCode, const QProcess::ExitStatus exitStatus, const QSharedPointer<QProcess> cmd);
    inline static QSharedPointer<QProcess> getProcess(Log::Log log, const QString& program, const QStringList& arg)
    {
        auto cmd = QSharedPointer<QProcess>::create();
        cmd->setProcessChannelMode(QProcess::MergedChannels);
        cmd->setProgram(program);
        cmd->setArguments(arg);
        QObject::connect(cmd.data(),
                         static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                         [cmd, log](int exitCode, QProcess::ExitStatus exitStatus) mutable
                         {
                             processFinishedHandler(log, exitCode, exitStatus, cmd);
                         });
        return cmd;
    }

    void getAllUsers(const QString& path = "");

private:
    static Manager* m_toolBoxManager;
    QString m_osUserInfoJson;
    QReadWriteLock* m_osUserNameMutex;
    QFileSystemWatcher* m_userNameWatcher;
    RealTimeAlert* m_realTimeAlert;
    Database* m_db;
};

};  // namespace ToolBox
};  // namespace KS