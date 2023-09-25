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

class FileProtectedAdaptor;

namespace KS
{
class FileProtected : public QObject,
                      protected QDBusContext
{
    Q_OBJECT
public:
    FileProtected(QObject *parent);
    virtual ~FileProtected();

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
    FileProtectedAdaptor *m_dbusAdaptor;
    Kss *m_kss;
};
}  // namespace KS
