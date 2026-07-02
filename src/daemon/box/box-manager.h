/**
 * @file          /ks-sc/src/daemon/box/box-manager.h
 * @brief         
 * @author        chendingjian <chendingjian@kylinos.com>
 * @copyright (c) 2023 KylinSec. All rights reserved.
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
    void DelBox(const QString &password, const QString &box_uid);
    QString GetBoxByUID(const QString &box_uid);
    QString GetBoxs();
    bool IsMounted(const QString &box_uid);
    void ModifyBoxPassword(const QString &box_uid, const QString &current_password, const QString &new_password);
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
