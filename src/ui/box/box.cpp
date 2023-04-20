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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/ui/box/box.h"
#include <qt5-log-i.h>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include "include/sc-marcos.h"
#include "lib/base/crypto-helper.h"
#include "sc-i.h"
#include "src/ui/box/modify-password.h"
#include "src/ui/box/retrieve-password.h"
#include "src/ui/box_manager_proxy.h"

namespace KS
{
Box::Box(const QString &uid) : m_uid(uid),
                               m_name("Unknown"),
                               m_mounted(false),
                               m_modifyPassword(nullptr),
                               m_retrievePassword(nullptr),
                               m_popupMenu(nullptr)
{
    this->m_boxManagerProxy = new BoxManagerProxy(SC_DBUS_NAME,
                                                  SC_BOX_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);
    m_process = new QProcess;
    m_imageLock = new BoxImage(this, ":/images/box-locked");
    m_imageUnlock = new BoxImage(this);

    this->initBox();
    this->initMenu();
}

Box::~Box()
{
    if (m_modifyPassword)
    {
        delete m_modifyPassword;
        m_modifyPassword = nullptr;
    }
    if (m_retrievePassword)
    {
        delete m_retrievePassword;
        m_retrievePassword = nullptr;
    }
    if (m_process)
    {
        delete m_process;
        m_process = nullptr;
    }
}

void Box::initBox()
{
    this->initBoxInfo();

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    this->m_showingIcon = new QPushButton(this);
    // 放在qss中会被scrollarea->viewport的样式表覆盖，所以在代码中设定背景
    this->m_showingIcon->setFlat(true);
    this->m_showingIcon->setFixedSize(QSize(102, 102));
    //    this->m_showingIcon->setIcon(QIcon(":/images/box-big"));
    //    this->m_showingIcon->setIconSize(QSize(70, 70));
    auto *vlay = new QVBoxLayout(m_showingIcon);
    vlay->setContentsMargins(0, 0, 0, 0);
    vlay->addWidget(m_imageLock, 0, Qt::AlignCenter);
    vlay->addWidget(m_imageUnlock, 0, Qt::AlignCenter);

    if (m_mounted)
    {
        m_imageLock->hide();
    }
    else
    {
        m_imageUnlock->hide();
    }

    vlay->addWidget(m_imageLock, 0, Qt::AlignCenter);

    layout->addWidget(this->m_showingIcon);

    connect(m_showingIcon, &QPushButton::clicked, this, &Box::onIconBtnClick);

    this->m_showingName = new QLabel(this);
    this->m_showingName->setText(this->m_name);
    this->m_showingName->setAlignment(Qt::AlignCenter);
    layout->addWidget(this->m_showingName);

    this->setLayout(layout);
}

void Box::initBoxInfo()
{
    auto reply = this->m_boxManagerProxy->GetBoxByUID(this->m_uid);
    auto jsonStr = reply.value();

    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser box information failed: " << jsonError.errorString();
        return;
    }

    auto jsonBox = jsonDoc.object();
    this->m_name = jsonBox.value(SCBM_JK_BOX_NAME).toString();
    this->m_mounted = QVariant(jsonBox.value(SCBM_JK_BOX_MOUNTED).toString()).toBool();
}

void Box::initMenu()
{
    auto mounted = this->m_boxManagerProxy->IsMounted(this->m_uid).value();

    this->m_popupMenu = new QMenu(this);
    this->m_popupMenu->setStyleSheet("font:NotoSansCJKsc-Regular;"
                                     "font-size:12px;"
                                     "color:black;");

    this->m_mountedStatusAction = this->m_popupMenu->addAction(mounted ? tr("Lock") : tr("Unlock"),
                                                               this,
                                                               &Box::switchMountedStatus);
    this->m_popupMenu->addAction(tr("Modify password"), this, &Box::modifyPassword);
    this->m_popupMenu->addAction(tr("Delete"), this, &Box::delBox);
    this->m_popupMenu->addAction(tr("Retrieve the password"), this, &Box::retrievePassword);
}

void Box::boxChanged()
{
    // 保密箱属性发生变化，需要进行更新
    auto mounted = this->m_boxManagerProxy->IsMounted(this->m_uid).value();
    m_mounted = mounted;
    this->m_mountedStatusAction->setText(mounted ? tr("Lock") : tr("Unlock"));
    if (m_mounted)
    {
        m_imageUnlock->show();
        m_imageLock->hide();
    }
    else
    {
        m_imageLock->show();
        m_imageUnlock->hide();
    }
}

void Box::onIconBtnClick()
{
    if (!m_mounted)
    {
        emit this->unUnlockedIconClicked();
        return;
    }

    QString path = QString("/box/%1/").arg(m_name);
    QString cmd = QString("caja %1").arg(path);
    KLOG_DEBUG() << "Open dir. path = " << path;
    m_process->start("bash", QStringList() << "-c" << cmd);
}

void Box::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        this->m_popupMenu->popup(event->globalPos());
    }

    QWidget::mousePressEvent(event);
}

void Box::switchMountedStatus()
{
    auto mounted = this->m_boxManagerProxy->IsMounted(this->m_uid).value();

    if (mounted)
    {
        this->m_boxManagerProxy->UnMount(this->m_uid).waitForFinished();
    }
    else
    {
        emit sigInputMountPasswd(this->m_uid);
    }
}

void Box::modifyPassword()
{
    if (this->m_modifyPassword)
    {
        emit this->showModifyPassword(this->m_modifyPassword);
        return;
    }

    this->m_modifyPassword = new ModifyPassword();
    this->m_modifyPassword->hide();
    connect(this->m_modifyPassword, SIGNAL(accepted()), this, SLOT(modifyPasswordAccepted()));

    this->m_modifyPassword->setBoxName(this->m_name);
    emit this->showModifyPassword(this->m_modifyPassword);
}

void Box::delBox()
{
    emit sigDelBox(this->m_uid);
}

void Box::retrievePassword()
{
    if (this->m_retrievePassword)
    {
        //        this->m_retrievePassword->show();
        emit this->showRetrievePassword(m_retrievePassword);
        return;
    }

    this->m_retrievePassword = new RetrievePassword();
    connect(this->m_retrievePassword, SIGNAL(accepted()), this, SLOT(retrievePasswordAccepted()));
    emit this->showRetrievePassword(m_retrievePassword);
}

void Box::checkMountPasswd(const QString &passwd, const QString &boxUID)
{
    RETURN_IF_TRUE(boxUID != this->m_uid)
    auto encryptPasswd = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), passwd);
    auto reply = this->m_boxManagerProxy->Mount(this->m_uid, encryptPasswd);
    reply.waitForFinished();
    RETURN_IF_FALSE(reply.isValid())

    auto ret = reply.value();
    if (!ret)
    {
        emit this->sigMountPasswdResult(false);
    }
    else
    {
        emit this->sigMountPasswdResult(true);
    }
}

void Box::checkDelPasswd(const QString &passwd, const QString &boxUID)
{
    RETURN_IF_TRUE(boxUID != this->m_uid)
    auto encryptPasswd = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), passwd);
    auto reply = this->m_boxManagerProxy->DelBox(this->m_uid, encryptPasswd);
    reply.waitForFinished();
    RETURN_IF_FALSE(reply.isValid())

    auto ret = reply.value();
    if (!ret)
    {
        emit this->sigDelPasswdResult(false);
    }
    else
    {
        emit this->sigDelPasswdResult(true);
    }
}

void Box::modifyPasswordAccepted()
{
    auto encryptCurrentPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), this->m_modifyPassword->getCurrentPassword());
    auto encryptNewPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), this->m_modifyPassword->getNewPassword());

    auto reply = this->m_boxManagerProxy->ModifyBoxPassword(this->m_uid,
                                                            encryptCurrentPassword,
                                                            encryptNewPassword);
    reply.waitForFinished();
    RETURN_IF_FALSE(reply.isValid())

    auto ret = reply.value();
    if (!ret)
    {
        emit this->sigModifyPasswdResult(false);
    }
    else
    {
        emit this->sigModifyPasswdResult(true);
    }
}

void Box::retrievePasswordAccepted()
{
    auto encryptNewPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), this->m_retrievePassword->getNewPassword());
    auto encryptPassphrase = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), this->m_retrievePassword->getPassphrase());

    auto reply = this->m_boxManagerProxy->RetrievePassword(this->m_uid,
                                                           encryptPassphrase,
                                                           encryptNewPassword);
    reply.waitForFinished();
    RETURN_IF_FALSE(reply.isValid())

    auto ret = reply.value();
    if (!ret)
    {
        emit this->sigRetrievePasswordResult(false);
    }
    else
    {
        emit this->sigRetrievePasswordResult(true);
    }
}
}  // namespace KS
