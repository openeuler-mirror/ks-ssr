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
enum TRUSTED_FILE_TYPE
{
    // 未知文件类型
    UNKNOWN_TYPE = 0,
    // 可执行文件
    EXECUTABLE_FILE,
    // 动态库
    DYNAMIC_LIBRARY,
    // 内核模块
    KERNEL_MODULE,
    // 可执行脚本
    EXECUTABLE_SCRIPT
};

class KssInit;

class Kss : public QObject
{
    Q_OBJECT
public:
    Kss(QObject *parent = nullptr);
    virtual ~Kss(){};

public:
    // 可信保护
    // 添加文件
    void addTrustedFile(const QString &filePath);
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

    // 搜索 传入要搜索的列表，此处fileList为一个json字符串，可通过get*Files()获取
    QString search(const QString &pathKey, const QString &fileList);
public Q_SLOTS:
    void onProcessExit(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void execute(const QString &cmd);
    void initData();

private slots:
    void initDataResults();

signals:
    void initFinished();

private:
    QProcess *m_process;
    QString m_processOutput;
    QString m_errorOutput;
    KssInit *m_kssInit;
    QSettings *m_ini;
};
}  // namespace KS
#endif  // KSS_H
