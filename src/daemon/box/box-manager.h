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
#include "src/daemon/box/box-dao.h"

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
    Q_PROPERTY(QString RSAPublicKey READ rSAPublicKey)
    QString rSAPublicKey() const { return m_rSAPublicKey; };

public Q_SLOTS:  // METHODS
    // 创建box
    QString CreateBox(const QString &name, const QString &password);
    // 删除box
    bool DelBox(const QString &password, const QString &box_uid);
    // 通过box uid获取box
    QString GetBoxByUID(const QString &box_uid);
    // 获取所有box信息
    QString GetBoxs();
    // 通过box uid获取mount状态
    bool IsMounted(const QString &box_uid);
    // 修改box的密码
    bool ModifyBoxPassword(const QString &box_uid, const QString &current_password, const QString &new_password);
    // 挂载box（解锁）
    bool Mount(const QString &box_uid, const QString &password);
    // 取消挂载（加锁）
    void UnMount(const QString &box_uid);
Q_SIGNALS:  // SIGNALS
    // box增加
    void BoxAdded(const QString &box_uid);
    // box信息改变
    void BoxChanged(const QString &box_uid);
    // 删除box
    void BoxDeleted(const QString &box_uid);

public Q_SLOTS:

private:
    void init();
    QString getRandBoxUid();
    QString getRandStr(uint length);
    QString getSendUserName(const uint &userUid);

private:
    BoxManagerAdaptor *m_dbusAdaptor;
    BoxDao *m_boxDao;

    EcryptFS *m_ecryptFS;

    QString m_rSAPublicKey;  // property
    QString m_rSAPrivateKey;
};
}  // namespace KS
