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
#include "src/ui/box/input-password.h"
#include "src/ui/box/modify-password.h"
#include "src/ui/box/retrieve-password.h"
#include "src/ui/box_manager_proxy.h"
#include "src/ui/common/sub-window.h"

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

    // m_modifyPassword = new ModifyPassword();

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
        //        QWidget *widget = buildNotifyPage(tr("Box is locked, please unlocked!"));
        //        widget->show();
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
        auto inputMountPasswd = new SubWindow(window());
        inputMountPasswd->setTitle(tr("Unlock"));
        inputMountPasswd->setFixedSize(300, 220);

        m_inputMountPassword = new InputPassword(inputMountPasswd);
        connect(m_inputMountPassword, &InputPassword::accepted, this, &Box::inputMountPasswordAccepted);
        connect(m_inputMountPassword, &InputPassword::accepted, inputMountPasswd, &SubWindow::close);
        connect(m_inputMountPassword, &InputPassword::rejected, inputMountPasswd, &SubWindow::close);

        inputMountPasswd->getContentLayout()->addWidget(m_inputMountPassword);

        int x = window()->x() / 2 + inputMountPasswd->width();
        int y = window()->y() / 2 + inputMountPasswd->height();
        inputMountPasswd->move(x, y);
        inputMountPasswd->show();
    }
}

void Box::modifyPassword()
{
    auto modifyPassword = new SubWindow(window());
    modifyPassword->setFixedSize(400, 400);
    modifyPassword->setTitle(tr("Modify password"));

    m_modifyPassword = new ModifyPassword(modifyPassword);
    connect(m_modifyPassword, SIGNAL(accepted()), this, SLOT(modifyPasswordAccepted()));
    connect(m_modifyPassword, &ModifyPassword::passwdInconsistent, this, [this]
            {
                auto messge = buildNotifyPage(QString(tr("Please confirm whether the password is consistent.")));
                messge->show();
            });
    connect(m_modifyPassword, &ModifyPassword::inputEmpty, this, [this]
            {
                auto messge = buildNotifyPage(tr("The input cannot be empty, please improve the information."));
                messge->show();
            });
    connect(m_modifyPassword, &ModifyPassword::accepted, modifyPassword, &SubWindow::close);
    connect(m_modifyPassword, &ModifyPassword::rejected, modifyPassword, &SubWindow::close);

    m_modifyPassword->setBoxName(m_name);
    //    m_modifyPassword->show();

    modifyPassword->getContentLayout()->addWidget(m_modifyPassword);

    int x = window()->x() + window()->width() / 4 + modifyPassword->width() / 4;
    int y = window()->y() + window()->height() / 4 + modifyPassword->height() / 8;
    modifyPassword->move(x, y);
    modifyPassword->show();
}

void Box::delBox()
{
    auto inputDelBoxPasswd = new SubWindow(window());
    inputDelBoxPasswd->setTitle(tr("Del box"));
    inputDelBoxPasswd->setFixedSize(300, 220);

    m_inputDelBoxPassword = new InputPassword(inputDelBoxPasswd);
    connect(m_inputDelBoxPassword, &InputPassword::accepted, this, &Box::inputDelBoxPasswordAccepted);
    connect(m_inputDelBoxPassword, &InputPassword::accepted, inputDelBoxPasswd, &SubWindow::close);
    connect(m_inputDelBoxPassword, &InputPassword::rejected, inputDelBoxPasswd, &SubWindow::close);

    inputDelBoxPasswd->getContentLayout()->addWidget(m_inputDelBoxPassword);

    int x = window()->x() / 2 + inputDelBoxPasswd->width();
    int y = window()->y() / 2 + inputDelBoxPasswd->height();
    inputDelBoxPasswd->move(x, y);
    inputDelBoxPasswd->show();
}

void Box::retrievePassword()
{
    auto retrievePasswd = new SubWindow(window());
    retrievePasswd->setFixedSize(300, 220);
    retrievePasswd->setTitle(tr("Retrieve password"));

    m_retrievePassword = new RetrievePassword(retrievePasswd);
    connect(m_retrievePassword, SIGNAL(accepted()), this, SLOT(retrievePasswordAccepted()));
    connect(m_retrievePassword, &RetrievePassword::inputEmpty, this, [this]
            {
                auto messge = buildNotifyPage(tr("The input cannot be empty, please improve the information."));
                messge->show();
            });
    connect(m_retrievePassword, &RetrievePassword::accepted, retrievePasswd, &SubWindow::close);
    connect(m_retrievePassword, &RetrievePassword::rejected, retrievePasswd, &SubWindow::close);

    retrievePasswd->getContentLayout()->addWidget(m_retrievePassword);

    int x = window()->x() + window()->width() / 4 + retrievePasswd->width() / 4;
    int y = window()->y() + window()->height() / 4 + retrievePasswd->height() / 8;
    retrievePasswd->move(x, y);
    retrievePasswd->show();
}

QWidget *Box::buildNotifyPage(const QString &notify)
{
    auto message = new SubWindow(window());
    message->setFixedSize(240, 200);
    message->buildNotify(notify);

    int x = window()->x() + window()->width() / 4 + message->width() / 4;
    int y = window()->y() + window()->height() / 4 + message->height() / 4;
    message->move(x, y);

    return message;
}

void Box::modifyPasswordAccepted()
{
    auto encryptCurrentPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_modifyPassword->getCurrentPassword());
    auto encryptNewPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_modifyPassword->getNewPassword());

    auto reply = m_boxManagerProxy->ModifyBoxPassword(m_uid,
                                                      encryptCurrentPassword,
                                                      encryptNewPassword);

    reply.waitForFinished();
    RETURN_IF_TRUE(!reply.isValid())

    if (!reply.value())
    {
        auto message = buildNotifyPage(QString(tr("Password error!")));
        message->show();
    }
    else
    {
        auto message = buildNotifyPage(QString(tr("Modify success!")));
        message->show();
    }
}

void Box::retrievePasswordAccepted()
{
    auto encryptPassphrase = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_retrievePassword->getPassphrase());

    auto reply = m_boxManagerProxy->RetrievePassword(m_uid, encryptPassphrase);
    reply.waitForFinished();
    RETURN_IF_TRUE(!reply.isValid())

    auto ret = reply.value();
    if (ret.isEmpty())
    {
        auto message = buildNotifyPage(QString(tr("Passphrase error!")));
        message->show();
    }
    else
    {
        auto message = buildNotifyPage(QString(tr("Your box password is %1")).arg(ret));
        message->show();
    }
}

void Box::inputMountPasswordAccepted()
{
    auto test = m_inputMountPassword->getInputPassword();
    auto encryptPasswd = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_inputMountPassword->getInputPassword());
    auto reply = m_boxManagerProxy->Mount(m_uid, encryptPasswd);
    reply.waitForFinished();
    RETURN_IF_TRUE(!reply.isValid())

    if (!reply.value())
    {
        auto message = buildNotifyPage(QString(tr("Password error!")));
        message->show();
    }
    else
    {
        auto message = buildNotifyPage(QString(tr("Unlock success!")));
        message->show();
    }
}

void Box::inputDelBoxPasswordAccepted()
{
    auto encryptPasswd = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_inputDelBoxPassword->getInputPassword());
    auto reply = m_boxManagerProxy->DelBox(m_uid, encryptPasswd);
    reply.waitForFinished();
    RETURN_IF_TRUE(!reply.isValid())

    if (!reply.value())
    {
        auto message = buildNotifyPage(QString(tr("The Password is wrong or has been mounted!")));
        message->show();
    }
    else
    {
        auto message = buildNotifyPage(QString(tr("Delete success!")));
        message->show();
    }
}
}  // namespace KS
