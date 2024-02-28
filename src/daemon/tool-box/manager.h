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
struct Group
{
    __gid_t gr_gid;     /* Group ID.	*/
    QString gr_name;    /* Group name.	*/
    QString gr_passwd;  /* Password.	*/
    QStringList gr_mem; /* Member list.	*/
};

struct Passwd
{
    QString pw_name;   /* Username.  */
    QString pw_passwd; /* Hashed passphrase, if shadow database
                        not in use (see shadow.h).  */
    __uid_t pw_uid;    /* User ID.  */
    __gid_t pw_gid;    /* Group ID.  */
    QString pw_gecos;  /* Real name.  */
    QString pw_dir;    /* Home directory.  */
    QString pw_shell;  /* Shell program.  */
};


class Manager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    enum class SeLabelType
    {
        MLS,
        KIC
    };
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
    QString GetFileMLSLabel(const QString& filePath);

    /**
     * @brief 设置安全上下文
     * @param filePath 文件路径
     * @param SecurityContext 需要设置的安全上下文
     */
    void SetFileMLSLabel(const QString& filePath, const QString& SecurityContext);

    QString GetFileKICLabel(const QString& filePath);

    void SetFileKICLabel(const QString& filePath, const QString& SecurityContext);

    QString GetUserMLSLabel(const QString& userName);

    void SetUserMLSLabel(const QString& userName, const QString& SecurityContext);

    /**
     * @brief 调用 shred 来彻底删除文件的函数
     * @param filePaths 需要删除的文件的路径
     */
    void ShredFile(const QStringList& filePaths);

    /**
     * @brief 删除用户及其相关数据
     * @param userName 要删除的用户名
     */
    void RemoveUser(const QStringList& userNames);

    bool GetAccessStatus();

    QString GetAllUsers();

    QStringList GetObjListFromSecuritySign();

    void AddObjToSecuritySign(const QStringList&);

    void RemoveObjFromSecuritySign(const QStringList&);

    QStringList GetFileListFromFileShred();

    void AddFileToFileShred(const QStringList&);

    void RemoveFileFromFileShred(const QStringList&);
    bool removeFileFromFileShred(const QStringList&);

    static void hazardDetected(uint, const QString&);

Q_SIGNALS:  // SIGNALS
    void FileShredListChanged();
    void FileSignListChanged();
    void UserChanged();
    void HazardDetected(uint type, const QString& alert_msg);

private:
    // 提权通过后执行
    void setAccessControlStatus(const QDBusMessage& message, bool enable);
    void removeUser(const QDBusMessage& message, const QStringList& userNames);
    void setFileMLSLabel(const QDBusMessage& message, const QString& filePath, const QString& SecurityContext);
    void setFileKICLabel(const QDBusMessage& message, const QString& filePath, const QString& SecurityContext);
    void setUserMLSLabel(const QDBusMessage& message, const QString& userName, const QString& SecurityContext);
    void shredFile(const QDBusMessage& message, const QStringList& filePaths);
    QStringList isPathsExist(const QStringList&);

    // 实际调用 Shred 命令的函数， 此函数的参数文件列表中应该只有普通文件。
    bool shred(const QStringList& filePaths);

private:
    Manager();
    virtual ~Manager() = default;
    void initDatabase();
    bool setFileSeLabels(const QString& filePath, const QString& seLabel, QString& output, const SeLabelType seLabelType);
    bool getFileSeLabels(const QString& filePath, QString& output, const SeLabelType SeLabelType);
    bool setUserSeLabels(const QString& userName, const QString& seLabel, QString& output);
    bool getUserSeLabels(const QString& userName, QString& output);
    static void processFinishedHandler(Log::Log log, const int exitCode, const QProcess::ExitStatus exitStatus, const QSharedPointer<QProcess> cmd);
    inline static QSharedPointer<QProcess> getProcess(const QString& program, const QStringList& arg, Log::Log log = Log::Log())
    {
        auto cmd = QSharedPointer<QProcess>::create();
        cmd->setProcessChannelMode(QProcess::MergedChannels);
        cmd->setProgram(program);
        cmd->setArguments(arg);
        if (!log.logMsg.isEmpty())
        {
            QObject::connect(cmd.data(),
                            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                            [cmd, log](int exitCode, QProcess::ExitStatus exitStatus) mutable
                            {
                                processFinishedHandler(log, exitCode, exitStatus, cmd);
                            });
        }
        return cmd;
    }

    void updateAccountInfo(const QString& path = "");

private:
    static Manager* m_toolBoxManager;
    QList<Group> m_osGroupInfo;
    QList<Passwd> m_osUserInfo;
    QString m_osUserInfoJson;
    QReadWriteLock* m_osUserNameMutex;
    QFileSystemWatcher* m_userNameWatcher;
    RealTimeAlert* m_realTimeAlert;
    Database* m_db;
};

};  // namespace ToolBox
};  // namespace KS
