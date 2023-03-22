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
#include "lib/base/base.h"

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
    std::string getRsaPrivateKey() { return m_rSAPrivateKey; };

public:  // PROPERTIES
    Q_PROPERTY(QString RSAPublicKey READ rSAPublicKey)
    QString rSAPublicKey() const { return QString::fromStdString(m_rSAPublicKey); };

public Q_SLOTS:  // METHODS
    QString CreateBox(const QString &name, const QString &password);
    bool DelBox(const QString &password, const QString &box_uid);
    QString GetBoxByUID(const QString &box_uid);
    QString GetBoxs();
    bool IsMounted(const QString &box_uid);
    bool ModifyBoxPassword(const QString &box_uid, const QString &current_password, const QString &new_password);
    bool Mount(const QString &box_uid, const QString &password);
    void UnMount(const QString &box_uid);
Q_SIGNALS:  // SIGNALS
    void BoxAdded(const QString &box_uid);
    void BoxChanged(const QString &box_uid);
    void BoxDeleted(const QString &box_uid);

public Q_SLOTS:

private:
    void init();

private:
    BoxManagerAdaptor *m_dbusAdaptor;

    EcryptFS *m_ecryptFS;

    std::string m_rSAPublicKey;  // property
    std::string m_rSAPrivateKey;
};
}  // namespace KS
