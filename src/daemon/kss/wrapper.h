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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#pragma once

#include <QObject>
#include <QProcess>
#include <QSettings>
#include "include/ssr-i.h"

namespace KS
{
namespace KSS
{
#define KSS_DEFAULT_USER_PIN "123123"

// 对kss命令进行封装
class Wrapper : public QObject
{
    Q_OBJECT
public:
    Wrapper(QObject *parent = nullptr);
    virtual ~Wrapper(){};
    static QSharedPointer<Wrapper> getDefault();

public:
    // 可信保护
    // 添加文件
    QString addTrustedFile(const QString &filePath);
    QString addTrustedFiles(const QString &json);
    // 移除文件
    void removeTrustedFile(const QString &filePath);
    void removeTrustedFiles(const QString &json);
    // 获取可信文件
    QString getTrustedFiles(SSRKSSTrustedFileType type);
    // 防卸载功能开关
    void prohibitUnloading(bool prohibited, const QString &filePath);
    // 获取程序白名单
    QString getExecuteFiles();

    // 可信切换软模式
    QString setStorageMode(SSRKSSTrustedStorageType type, const QString &userPin);
    SSRKSSTrustedStorageType getCurrentStorageMode();
    // 可信开关
    void setTrustedStatus(bool status);
    QString getTrustedStatus();

    // 文件保护
    // 添加文件
    void addFile(const QString &fileName, const QString &filePath, const QString &insertTime);
    void addFiles(const QString &json);
    // 移除文件
    void removeFile(const QString &filePath);
    void removeFiles(const QString &json);
    // 获取文件保护列表
    QString getFiles();

    int getInitialized();

public Q_SLOTS:
    void
    processExited(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void init();
    void execute(const QString &cmd);

private slots:
    void initTrustedResults();

signals:
    void initFinished();

private:
    static QSharedPointer<Wrapper> m_instance;

    QProcess *m_process;
    QString m_processOutput;
    QString m_errorOutput;
    QSettings *m_ini;
    QThread *m_kssInitThread;
};
}  // namespace KSS
}  // namespace KS
