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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#pragma once

#include <QDBusContext>
#include <QProcess>
#include <QSharedPointer>

class QFileSystemWatcher;
class QReadWriteLock;
class QSettings;

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
    __gid_t gid;     /* Group ID.	*/
    QString name;    /* Group name.	*/
    QString passwd;  /* Password.	*/
    QStringList mem; /* Member list.	*/
};

struct Passwd
{
    QString name;   /* Username.  */
    QString passwd; /* Hashed passphrase, if shadow database
                        not in use (see shadow.h).  */
    __uid_t uid;    /* User ID.  */
    __gid_t gid;    /* Group ID.  */
    QString gecos;  /* Real name.  */
    QString dir;    /* Home directory.  */
    QString shell;  /* Shell program.  */
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

    bool removeObjFromSecuritySign(const QStringList&);

    QStringList GetFileListFromFileShred();

    void AddFileToFileShred(const QStringList&);

    void RemoveFileFromFileShred(const QStringList&);
    bool removeFileFromFileShred(const QStringList&);

    static void hazardDetected(uint, const QString&);

    /**
     * @brief UID 是否可复用
     * @param enabled 开关状态
     * @note 请注意使用 QMutexLocker 来避免多线程的问题。
     */
    void SetUidReusable(bool enabled);

    bool GetUidReusable();
    void SetMultiFactorAuthState(bool enabled);
    bool GetMultiFactorAuthState();

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
    inline static QSharedPointer<QProcess> getProcess(const QString& program, const QStringList& arg)
    {
        auto cmd = QSharedPointer<QProcess>::create();
        cmd->setProcessChannelMode(QProcess::MergedChannels);
        cmd->setProgram(program);
        cmd->setArguments(arg);
        return cmd;
    }

    void updateAccountInfo(const QString& path = "");

    bool getMultiFactorAuthState();
    void disableMultiFactorAuthState();
    void enableMultiFactorAuthState();
    void disableAuthType(QList<int>);
    void enableAuthType(QList<int>);

private:
    static Manager* m_toolBoxManager;
    QList<Group> m_osGroupInfo;
    QMap<QString, Passwd> m_osUserInfo;
    QString m_osUserInfoJson;
    QReadWriteLock* m_osUserNameMutex;
    QFileSystemWatcher* m_userNameWatcher;
    RealTimeAlert* m_realTimeAlert;
    Database* m_db;

    /**
     * @brief uid 是否可复用的配置文件， 路径默认是 UID_REUSE_CONTROL_PATH
     */
    QSettings* m_uidReuseConfig;

    /**
     * @brief UID 是否可复用
     */
    bool m_isUidReusable;
    bool m_multiFactorAuthState;
};

};  // namespace ToolBox
};  // namespace KS
