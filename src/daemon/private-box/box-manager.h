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

#include <QDBusContext>
#include <QDBusObjectPath>
#include <QDBusServiceWatcher>
#include <QObject>
#include <QStringList>
#include "ecryptfs.h"
#include "src/daemon/private-box/box.h"

class BoxManagerAdaptor;

namespace KS
{
namespace PrivateBox
{
class BoxManager : public QObject,
                   protected QDBusContext
{
    Q_OBJECT

public:
    static void globalInit(QObject *parent);
    static void globalDeinit();

    static BoxManager *instance()
    {
        return m_instance;
    };

public:  // PROPERTIES
    Q_PROPERTY(QString RSAPublicKey READ rsaPublicKey)
    QString rsaPublicKey() const
    {
        return m_rsaPublicKey;
    };

public Q_SLOTS:  // METHODS
    // 创建box
    QString CreateBox(const QString &name, const QString &password, QString &passphrase);
    // 删除box
    void DelBox(const QString &boxID, const QString &password);
    // 通过box uid获取box
    QString GetBoxByUID(const QString &boxID);
    // 获取所有box信息
    QString GetBoxs();
    // 通过box uid获取mount状态
    bool IsMounted(const QString &boxID);
    // 修改box的密码
    void ModifyBoxPassword(const QString &boxID,
                           const QString &currentPassword,
                           const QString &newPassword);
    // 挂载box（解锁）
    void Mount(const QString &boxID, const QString &password);
    // 通过口令找回密码
    QString RetrieveBoxPassword(const QString &boxID, const QString &passphrase);
    // 取消挂载（加锁）
    void UnMount(const QString &boxID);
Q_SIGNALS:  // SIGNALS
    // box增加
    void BoxAdded(const QString &boxID, const QString &passphrase);
    // box信息改变
    void BoxChanged(const QString &boxID);
    // 删除box
    void BoxDeleted(const QString &boxID);

private:
    BoxManager(QObject *parent);
    virtual ~BoxManager();

private:
    void init();
    uint getSenderUid();

private slots:
    void unMountAllBoxs(const QString &service);

private:
    static BoxManager *m_instance;

    BoxManagerAdaptor *m_dbusAdaptor;
    QDBusServiceWatcher *m_serviceWatcher;
    QMap<QString, Box *> m_boxs;

    QString m_rsaPublicKey;  // property
    QString m_rsaPrivateKey;
};
}  // namespace PrivateBox
}  // namespace KS
