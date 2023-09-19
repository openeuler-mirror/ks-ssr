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
#include "ecryptfs.h"
#include "src/daemon/box/box.h"

class BoxManagerAdaptor;

namespace KS
{
class BoxManager : public QObject,
                   protected QDBusContext
{
    Q_OBJECT
public:
    BoxManager(QObject *parent);
    virtual ~BoxManager();

public:  // PROPERTIES
    Q_PROPERTY(QString RSAPublicKey READ rsaPublicKey)
    QString rsaPublicKey() const { return m_rsaPublicKey; };

public Q_SLOTS:  // METHODS
    // 创建box
    QString CreateBox(const QString &name, const QString &password);
    // 删除box
    bool DelBox(const QString &password, const QString &boxId);
    // 通过box uid获取box
    QString GetBoxByUID(const QString &boxId);
    // 获取所有box信息
    QString GetBoxs();
    // 通过box uid获取mount状态
    bool IsMounted(const QString &boxId);
    // 修改box的密码
    bool ModifyBoxPassword(const QString &boxId,
                           const QString &currentPassword,
                           const QString &newPassword);
    // 挂载box（解锁）
    bool Mount(const QString &boxId, const QString &password);
    // 通过口令找回密码
    bool RetrievePassword(const QString &boxId, const QString &passphrase, const QString &newPassword);
    // 取消挂载（加锁）
    void UnMount(const QString &boxId);
Q_SIGNALS:  // SIGNALS
    // box增加
    void BoxAdded(const QString &boxId, const QString &passphrase);
    // box信息改变
    void BoxChanged(const QString &boxId);
    // 删除box
    void BoxDeleted(const QString &boxId);

public Q_SLOTS:

private:
    void init();
    uint getSenderUid();

private:
    BoxManagerAdaptor *m_dbusAdaptor;
    QMap<QString, Box *> m_boxs;

    QString m_rsaPublicKey;  // property
    QString m_rsaPrivateKey;
};
}  // namespace KS