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
#pragma once

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
    Q_PROPERTY(bool Initialized READ initialized)
    bool initialized() const;

    Q_PROPERTY(uint StorageMode READ storageMode)
    uint storageMode() const;

    Q_PROPERTY(bool TrustedStatus READ trustedStatus)
    bool trustedStatus() const;

public Q_SLOTS:  // METHODS
    // 添加文件保护文件
    void AddProtectedFile(const QString &filePath);
    // 添加文件保护文件列表
    void AddProtectedFiles(const QStringList &fileList);
    // 添加可信保护文件
    void AddTrustedFile(const QString &filePath);
    // 添加可信保护文件列表
    void AddTrustedFiles(const QStringList &fileList);
    // 获取可信保护列表 @type ：可信类型(kernel/excute)
    QString GetTrustedFiles(uint type);
    // 获取文件保护列表
    QString GetProtectedFiles();
    // 设置防卸载
    void ProhibitUnloading(bool prohibited, const QString &filePath);
    // 移除文件保护文件
    void RemoveProtectedFile(const QString &filePath);
    // 移除文件保护文件列表
    void RemoveProtectedFiles(const QStringList &fileList);
    // 移除可信保护文件
    void RemoveTrustedFile(const QString &filePath);
    // 移除可信保护文件列表
    void RemoveTrustedFiles(const QStringList &fileList);
    // 存储模式切换
    void SetStorageMode(uint type, const QString &userPin);
    // 设置可信状态
    void SetTrustedStatus(bool status);

    // 搜索 暂时保留接口 未使用
    QString Search(const QString &path_key, uint searchType);
Q_SIGNALS:  // SIGNALS
    void InitFinished();
    void TrustedFilesChange();
    void ProtectedFilesChange();

private:
    void init();

private:
    void addTPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath);
    void addTPFilesAfterAuthorization(const QDBusMessage &message, const QStringList &fileList);
    void removeTPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath);
    void removeTPFilesAfterAuthorization(const QDBusMessage &message, const QStringList &fileList);
    void prohibitUnloadingAfterAuthorization(const QDBusMessage &message, bool prohibited, const QString &filePath);
    void addFPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath);
    void addFPFilesAfterAuthorization(const QDBusMessage &message, const QStringList &fileList);
    void removeFPFileAfterAuthorization(const QDBusMessage &message, const QString &filePath);
    void removeFPFilesAfterAuthorization(const QDBusMessage &message, const QStringList &fileList);
    void setStorageModeAfterAuthorization(const QDBusMessage &message, uint type, const QString &userPin);
    void setTrustedStatusAfterAuthorization(const QDBusMessage &message, bool status);

private:
    KSSDbusAdaptor *m_dbusAdaptor;
};

}  // namespace KS
