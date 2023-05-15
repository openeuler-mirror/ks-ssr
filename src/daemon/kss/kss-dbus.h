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
#ifndef KSSDBUS_H
#define KSSDBUS_H

#include <QDBusContext>
#include <QDBusObjectPath>
#include <QObject>

class KSSDbusAdaptor;

namespace KS
{
enum KSSType
{
    KSS_TYPE_TP_EXECUTE = 0,
    KSS_TYPE_TP_KERNEL,
    KSS_TYPE_FP,
    KSS_TYPE_NONE
};

class KSSDbus : public QObject,
                protected QDBusContext
{
    Q_OBJECT
public:
    KSSDbus(QObject *parent = nullptr);
    virtual ~KSSDbus(){};

public:  // PROPERTIES
    Q_PROPERTY(int Initialized READ initialized)
    int initialized() const;

public Q_SLOTS:  // METHODS
    // 添加文件保护文件
    void AddFPFile(const QString &filePath);
    // 添加可信保护文件
    void AddTPFile(const QString &filePath);
    // 获取执行保护列表
    QString GetExecuteFiles();
    // 获取文件保护列表
    QString GetFPFiles();
    // 获取内核保护列表
    QString GetModuleFiles();
    // 设置防卸载
    void ProhibitUnloading(bool prohibited, const QString &filePath);
    // 移除文件保护文件
    void RemoveFPFile(const QString &filePath);
    // 移除可信保护文件
    void RemoveTPFile(const QString &filePath);
    // 搜索 暂时保留接口 未使用
    QString Search(const QString &path_key, uint searchType);
Q_SIGNALS:  // SIGNALS
    void InitFinished();

private:
    void init();

private:
    void addTPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath);
    void removeTPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath);
    void prohibitUnloadingAfterAuthorization(const QDBusMessage &message, bool prohibited, const QString &filePath);
    void addFPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath);
    void removeFPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath);

private:
    KSSDbusAdaptor *m_dbusAdaptor;
};

}  // namespace KS
#endif  // KSSDBUS_H
