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

namespace KS
{
namespace ToolBox
{

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
    void SherdFile(const QStringList& filePath);

    /**
     * @brief 删除用户及其相关数据
     * @param userName 要删除的用户名
     */
    void RemoveUser(const QStringList& userNames);

private:
    Manager();
    virtual ~Manager() = default;
    static void processFinishedHandler(const int exitCode, const QProcess::ExitStatus exitStatus, const QSharedPointer<QProcess> cmd);

    inline static QSharedPointer<QProcess> getProcess(const QString& program, const QStringList& arg)
    {
        auto cmd = QSharedPointer<QProcess>::create();
        cmd->setProcessChannelMode(QProcess::MergedChannels);
        cmd->setProgram(program);
        cmd->setArguments(arg);
        QObject::connect(cmd.data(),
                         static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                         [cmd](int exitCode, QProcess::ExitStatus exitStatus) {
                             processFinishedHandler(exitCode, exitStatus, cmd);
                         });
        return cmd;
    }

private:
    static Manager* m_toolBoxManager;
};

};  // namespace ToolBox
};  // namespace KS