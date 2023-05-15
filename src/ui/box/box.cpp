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
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "lib/base/crypto-helper.h"
#include "src/ui/box/box-password-checked.h"
#include "src/ui/box/box-password-modification.h"
#include "src/ui/box/box-password-retrieve.h"
#include "src/ui/box_manager_proxy.h"
#include "src/ui/common/message-dialog.h"

namespace KS
{
Box::Box(const QString &uid) : m_uid(uid),
                               m_name("Unknown"),
                               m_mounted(false),
                               m_modifyPassword(nullptr),
                               m_retrievePassword(nullptr),
                               m_popupMenu(nullptr)
{
    m_boxManagerProxy = new BoxManagerProxy(KSC_DBUS_NAME,
                                            KSC_BOX_MANAGER_DBUS_OBJECT_PATH,
                                            QDBusConnection::systemBus(),
                                            this);
    m_process = new QProcess;
    m_imageLock = new BoxImage(this, ":/images/box-locked");
    m_imageUnlock = new BoxImage(this);

    initBox();
    initMenu();
}

void Box::initBox()
{
    initBoxInfo();

    /* setFixedWidth(102); */

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_showingIcon = new QPushButton(this);
    // 放在qss中会被scrollarea->viewport的样式表覆盖，所以在代码中设定背景
    m_showingIcon->setFlat(true);
    m_showingIcon->setFixedSize(102, 102);
    //    m_showingIcon->setIcon(QIcon(":/images/box-big"));
    //    m_showingIcon->setIconSize(QSize(70, 70));
    QVBoxLayout *vlay = new QVBoxLayout(m_showingIcon);
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

    layout->addWidget(m_showingIcon);

    connect(m_showingIcon, &QPushButton::clicked, this, &Box::onIconBtnClick);

    m_showingName = new QLabel(this);
    m_showingName->setText(m_name);
    m_showingName->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_showingName);

    setLayout(layout);
}

void Box::initBoxInfo()
{
    auto reply = m_boxManagerProxy->GetBoxByUID(m_uid);
    auto jsonStr = reply.value();

    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser box information failed: " << jsonError.errorString();
        return;
    }

    auto jsonBox = jsonDoc.object();
    m_name = jsonBox.value(KSC_BM_JK_BOX_NAME).toString();
    m_mounted = jsonBox.value(KSC_BM_JK_BOX_MOUNTED).toBool();
}

void Box::initMenu()
{
    auto mounted = m_boxManagerProxy->IsMounted(m_uid).value();

    // m_modifyPassword = new BoxPasswordModification();

    m_popupMenu = new QMenu(this);
    m_popupMenu->setObjectName("m_popupMenu");

    m_mountedStatusAction = m_popupMenu->addAction(mounted ? tr("Lock") : tr("Unlock"),
                                                   this,
                                                   &Box::switchMountedStatus);
    m_popupMenu->addAction(tr("Modify password"), this, &Box::modifyPassword);
    m_popupMenu->addAction(tr("Delete"), this, &Box::delBox);
    m_popupMenu->addAction(tr("Retrieve the password"), this, &Box::retrievePassword);
}

void Box::boxChanged()
{
    // 保密箱属性发生变化，需要进行更新
    auto mounted = m_boxManagerProxy->IsMounted(m_uid).value();
    m_mounted = mounted;
    m_mountedStatusAction->setText(mounted ? tr("Lock") : tr("Unlock"));
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
        switchMountedStatus();
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
        m_popupMenu->popup(event->globalPos());
    }

    QWidget::mousePressEvent(event);
}

void Box::switchMountedStatus()
{
    auto mounted = m_boxManagerProxy->IsMounted(m_uid).value();
    if (mounted)
    {
        m_boxManagerProxy->UnMount(m_uid).waitForFinished();
    }
    else
    {
        m_inputMountPassword = new BoxPasswordChecked(window());
        m_inputMountPassword->setTitle(tr("Unlock"));
        m_inputMountPassword->setFixedSize(300, 240);
        connect(m_inputMountPassword, &BoxPasswordChecked::accepted, this, &Box::inputMountPasswordAccepted);

        int x = window()->x() / 2 + m_inputMountPassword->width();
        int y = window()->y() / 2 + m_inputMountPassword->height();
        m_inputMountPassword->move(x, y);
        m_inputMountPassword->show();
    }
}

void Box::modifyPassword()
{
    m_modifyPassword = new BoxPasswordModification(window());
    m_modifyPassword->setFixedSize(400, 400);
    m_modifyPassword->setTitle(tr("Modify password"));

    connect(m_modifyPassword, SIGNAL(accepted()), this, SLOT(modifyPasswordAccepted()));
    connect(m_modifyPassword, &BoxPasswordModification::passwdInconsistent, this, [this]
            {
                auto message = buildMessageDialog(QString(tr("Please confirm whether the password is consistent.")));
                message->show();
            });
    connect(m_modifyPassword, &BoxPasswordModification::inputEmpty, this, [this]
            {
                auto message = buildMessageDialog(tr("The input cannot be empty, please improve the information."));
                message->show();
            });

    m_modifyPassword->setBoxName(m_name);

    int x = window()->x() + window()->width() / 4 + m_modifyPassword->width() / 4;
    int y = window()->y() + window()->height() / 4 + m_modifyPassword->height() / 8;
    m_modifyPassword->move(x, y);
    m_modifyPassword->show();
}

void Box::delBox()
{
    m_inputDelBoxPassword = new BoxPasswordChecked(window());
    m_inputDelBoxPassword->setTitle(tr("Del box"));
    m_inputDelBoxPassword->setFixedSize(300, 240);
    connect(m_inputDelBoxPassword, &BoxPasswordChecked::accepted, this, &Box::inputDelBoxPasswordAccepted);

    int x = window()->x() / 2 + m_inputDelBoxPassword->width();
    int y = window()->y() / 2 + m_inputDelBoxPassword->height();
    m_inputDelBoxPassword->move(x, y);
    m_inputDelBoxPassword->show();
}

void Box::retrievePassword()
{
    m_retrievePassword = new RetrieveBoxPassword(window());
    m_retrievePassword->setFixedSize(300, 220);
    m_retrievePassword->setTitle(tr("Retrieve password"));
    connect(m_retrievePassword, SIGNAL(accepted()), this, SLOT(retrievePasswordAccepted()));
    connect(m_retrievePassword, &RetrieveBoxPassword::inputEmpty, this, [this]
            {
                auto message = buildMessageDialog(tr("The input cannot be empty, please improve the information."));
                message->show();
            });

    int x = window()->x() + window()->width() / 4 + m_retrievePassword->width() / 4;
    int y = window()->y() + window()->height() / 4 + m_retrievePassword->height() / 8;
    m_retrievePassword->move(x, y);
    m_retrievePassword->show();
}

QWidget *Box::buildMessageDialog(const QString &message)
{
    // 此处设置了WA_DeleteOnClose的属性，message页面close时将销毁
    auto messageDialog = new MessageDialog(window());
    messageDialog->setMessage(message);

    int x = window()->x() + window()->width() / 4 + messageDialog->width() / 4;
    int y = window()->y() + window()->height() / 4 + messageDialog->height() / 4;
    messageDialog->move(x, y);

    return messageDialog;
}

void Box::modifyPasswordAccepted()
{
    auto encryptCurrentPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_modifyPassword->getCurrentPassword());
    auto encryptNewPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_modifyPassword->getNewPassword());

    auto reply = m_boxManagerProxy->ModifyBoxPassword(m_uid,
                                                      encryptCurrentPassword,
                                                      encryptNewPassword);

    reply.waitForFinished();

    auto message = buildMessageDialog(QString(reply.isError() ? reply.error().message() : tr("Modify success!")));
    message->show();
}

void Box::retrievePasswordAccepted()
{
    auto encryptPassphrase = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_retrievePassword->getPassphrase());

    auto reply = m_boxManagerProxy->RetrieveBoxPassword(m_uid, encryptPassphrase);
    reply.waitForFinished();

    auto message = buildMessageDialog(reply.isError() ? reply.error().message() : QString(tr("Your box password is %1")).arg(reply.value()));
    message->show();
}

void Box::inputMountPasswordAccepted()
{
    auto encryptPasswd = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_inputMountPassword->getBoxPasswordChecked());
    auto reply = m_boxManagerProxy->Mount(m_uid, encryptPasswd);
    reply.waitForFinished();

    auto message = buildMessageDialog(QString(reply.isError() ? reply.error().message() : tr("Unlock success!")));
    message->show();
}

void Box::inputDelBoxPasswordAccepted()
{
    auto encryptPasswd = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_inputDelBoxPassword->getBoxPasswordChecked());
    auto reply = m_boxManagerProxy->DelBox(m_uid, encryptPasswd);
    reply.waitForFinished();

    auto message = buildMessageDialog(QString(reply.isError() ? reply.error().message() : tr("Delete success!")));
    message->show();
}
}  // namespace KS
