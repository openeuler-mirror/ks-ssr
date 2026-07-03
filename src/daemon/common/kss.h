/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
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

#ifndef KSS_H
#define KSS_H

#include <QObject>
#include <QProcess>
#include <QSettings>

namespace KS
{
class KSS : public QObject
{
    Q_OBJECT
public:
    KSS(QObject *parent = nullptr);
    virtual ~KSS(){};
    static QSharedPointer<KSS> getDefault();

public:
    // 可信保护
    // 添加文件
    QString addTrustedFile(const QString &filePath);
    // 移除文件
    void removeTrustedFile(const QString &filePath);
    // 获取内核白名单
    QString getModuleFiles();
    // 防卸载功能开关
    void prohibitUnloading(bool prohibited, const QString &filePath);
    // 获取程序白名单
    QString getExecuteFiles();

    // 文件保护
    // 添加文件
    void addFile(const QString &fileName, const QString &filePath, const QString &insertTime);
    // 移除文件
    void removeFile(const QString &filePath);
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
    static QSharedPointer<KSS> m_instance;

    QProcess *m_process;
    QString m_processOutput;
    QString m_errorOutput;
    QSettings *m_ini;
    QThread *m_kssInitThread;
};
}  // namespace KS
#endif  // KSS_H
