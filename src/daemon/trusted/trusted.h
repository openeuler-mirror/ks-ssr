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
#include <QStringList>
#include "src/daemon/common/kss.h"

class TrustedAdaptor;

namespace KS
{
enum TrustedProtectType
{
    TRUSTED_PROTECT_EXECUTE = 0,
    TRUSTED_PROTECT_KERNEL,
    TRUSTED_PROTECT_NONE
};

class Trusted : public QObject,
                protected QDBusContext
{
    Q_OBJECT
public:
    Trusted(QObject *parent);
    virtual ~Trusted();

public:          // PROPERTIES
public Q_SLOTS:  // METHODS
    // 添加文件
    void AddFile(const QString &filePath);
    // 获取程序白名单
    QString GetExecuteFiles();
    // 获取内核白名单
    QString GetModuleFiles();
    // 防卸载功能开关
    void ProhibitUnloading(bool prohibited, const QString &filePath);
    // 移除文件
    void RemoveFile(const QString &filePath);
    // 搜索
    QString Search(const QString &pathKey, uint searchType);
Q_SIGNALS:  // SIGNALS
    // 初始化完成
    void InitFinished();

private:
    void init();

private:
    TrustedAdaptor *m_dbusAdaptor;
    KSS *m_kss;
};
}  // namespace KS
