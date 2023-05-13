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

class FPAdaptor;

namespace KS
{
class FP : public QObject,
           protected QDBusContext
{
    Q_OBJECT
public:
    FP(QObject *parent);
    virtual ~FP();

public:  // PROPERTIES
    Q_PROPERTY(int Initialized READ getInitialized)
    int getInitialized() const;

public Q_SLOTS:  // METHODS
    // 添加文件
    void AddFile(const QString &filePath);
    // 获取文件列表
    QString GetFiles();
    // 移除文件
    void RemoveFile(const QString &filePath);
    // 搜索
    QString Search(const QString &pathKey);

private:
    void init();

private:
    void onAddFile(const QDBusMessage &message, const QString &filePath);
    void onRemoveFile(const QDBusMessage &message, const QString &filePath);

private:
    FPAdaptor *m_dbusAdaptor;
};
}  // namespace KS
