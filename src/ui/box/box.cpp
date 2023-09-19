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
    this->m_boxManagerProxy = new BoxManagerProxy(KSC_DBUS_NAME,
                                                  KSC_BOX_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);
    m_process = new QProcess;
    m_imageLock = new BoxImage(this, ":/images/box-locked");
    m_imageUnlock = new BoxImage(this);

    this->initBox();
    this->initMenu();
}

void Box::initBox()
{
    this->initBoxInfo();

    /* this->setFixedWidth(102); */

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    this->m_showingIcon = new QPushButton(this);
    // 放在qss中会被scrollarea->viewport的样式表覆盖，所以在代码中设定背景
    this->m_showingIcon->setFlat(true);
    this->m_showingIcon->setFixedSize(QSize(102, 102));
    //    this->m_showingIcon->setIcon(QIcon(":/images/box-big"));
    //    this->m_showingIcon->setIconSize(QSize(70, 70));
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
    m_name = jsonBox.value(KSC_BM_JK_BOX_NAME).toString();
    m_mounted = QVariant(jsonBox.value(KSC_BM_JK_BOX_MOUNTED).toString()).toBool();
}

void Box::initMenu()
{
    auto mounted = this->m_boxManagerProxy->IsMounted(this->m_uid).value();

    // this->m_modifyPassword = new ModifyPassword();

    this->m_popupMenu = new QMenu(this);
    m_popupMenu->setObjectName("m_popupMenu");

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
        auto inputMountPasswd = new SubWindow(this->window());
        inputMountPasswd->setTitle(tr("Unlock"));
        inputMountPasswd->setFixedSize(300, 220);

        m_inputMountPassword = new InputPassword(inputMountPasswd);
        connect(m_inputMountPassword, &InputPassword::accepted, this, &Box::inputMountPasswordAccepted);
        connect(m_inputMountPassword, &InputPassword::accepted, inputMountPasswd, &SubWindow::close);
        connect(m_inputMountPassword, &InputPassword::rejected, inputMountPasswd, &SubWindow::close);

        inputMountPasswd->getContentLayout()->addWidget(m_inputMountPassword);

        int x = this->window()->x() / 2 + inputMountPasswd->width();
        int y = this->window()->y() / 2 + inputMountPasswd->height();
        inputMountPasswd->move(x, y);
        inputMountPasswd->show();
    }
}

void Box::modifyPassword()
{
    auto modifyPassword = new SubWindow(this->window());
    modifyPassword->setFixedSize(400, 320);
    modifyPassword->setTitle(tr("Modify password"));

    this->m_modifyPassword = new ModifyPassword(modifyPassword);
    connect(this->m_modifyPassword, SIGNAL(accepted()), this, SLOT(modifyPasswordAccepted()));
    connect(this->m_modifyPassword, &ModifyPassword::passwdInconsistent, this, [this]
            {
                auto messge = buildNotifyPage(QString(tr("Please confirm whether the password is consistent.")));
                messge->show();
            });
    connect(this->m_modifyPassword, &ModifyPassword::inputEmpty, this, [this]
            {
                auto messge = buildNotifyPage(tr("The input cannot be empty, please improve the information."));
                messge->show();
            });
    connect(this->m_modifyPassword, &ModifyPassword::accepted, modifyPassword, &SubWindow::close);
    connect(this->m_modifyPassword, &ModifyPassword::rejected, modifyPassword, &SubWindow::close);

    this->m_modifyPassword->setBoxName(this->m_name);
    //    this->m_modifyPassword->show();

    modifyPassword->getContentLayout()->addWidget(m_modifyPassword);

    int x = this->window()->x() + this->window()->width() / 4 + modifyPassword->width() / 4;
    int y = this->window()->y() + this->window()->height() / 4 + modifyPassword->height() / 4;
    modifyPassword->move(x, y);
    modifyPassword->show();
}

void Box::delBox()
{
    auto inputDelBoxPasswd = new SubWindow(this->window());
    inputDelBoxPasswd->setTitle(tr("Del box"));
    inputDelBoxPasswd->setFixedSize(300, 220);

    m_inputDelBoxPassword = new InputPassword(inputDelBoxPasswd);
    connect(m_inputDelBoxPassword, &InputPassword::accepted, this, &Box::inputDelBoxPasswordAccepted);
    connect(m_inputDelBoxPassword, &InputPassword::accepted, inputDelBoxPasswd, &SubWindow::close);
    connect(m_inputDelBoxPassword, &InputPassword::rejected, inputDelBoxPasswd, &SubWindow::close);

    inputDelBoxPasswd->getContentLayout()->addWidget(m_inputDelBoxPassword);

    int x = this->window()->x() / 2 + inputDelBoxPasswd->width();
    int y = this->window()->y() / 2 + inputDelBoxPasswd->height();
    inputDelBoxPasswd->move(x, y);
    inputDelBoxPasswd->show();
}

void Box::retrievePassword()
{
    auto retrievePasswd = new SubWindow(this->window());
    retrievePasswd->setFixedSize(400, 320);
    retrievePasswd->setTitle(tr("Modify password"));

    this->m_retrievePassword = new RetrievePassword(retrievePasswd);
    connect(this->m_retrievePassword, SIGNAL(accepted()), this, SLOT(retrievePasswordAccepted()));
    connect(this->m_retrievePassword, &RetrievePassword::passwdInconsistent, this, [this]
            {
                auto messge = buildNotifyPage(QString(tr("Please confirm whether the password is consistent.")));
                messge->show();
            });
    connect(this->m_retrievePassword, &RetrievePassword::inputEmpty, this, [this]
            {
                auto messge = buildNotifyPage(tr("The input cannot be empty, please improve the information."));
                messge->show();
            });
    connect(this->m_retrievePassword, &RetrievePassword::accepted, retrievePasswd, &SubWindow::close);
    connect(this->m_retrievePassword, &RetrievePassword::rejected, retrievePasswd, &SubWindow::close);

    //    this->m_modifyPassword->show();

    retrievePasswd->getContentLayout()->addWidget(m_retrievePassword);

    int x = this->window()->x() + this->window()->width() / 4 + retrievePasswd->width() / 4;
    int y = this->window()->y() + this->window()->height() / 4 + retrievePasswd->height() / 4;
    retrievePasswd->move(x, y);
    retrievePasswd->show();
}

QWidget *Box::buildNotifyPage(const QString &notify)
{
    auto message = new SubWindow(this->window());
    message->setFixedSize(240, 180);
    message->buildNotify(notify);

    int x = this->window()->x() + this->window()->width() / 4 + message->width() / 4;
    int y = this->window()->y() + this->window()->height() / 4 + message->height() / 4;
    message->move(x, y);

    return message;
}

void Box::modifyPasswordAccepted()
{
    auto encryptCurrentPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), this->m_modifyPassword->getCurrentPassword());
    auto encryptNewPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), this->m_modifyPassword->getNewPassword());

    auto reply = this->m_boxManagerProxy->ModifyBoxPassword(this->m_uid,
                                                            encryptCurrentPassword,
                                                            encryptNewPassword);

    bool ret = false;
    reply.waitForFinished();
    if (reply.isValid())
    {
        ret = reply.value();
        if (!ret)
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
    else
    {
        KLOG_WARNING() << "modify password failed. ret = " << ret;
    }
}

void Box::retrievePasswordAccepted()
{
    auto encryptNewPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), this->m_retrievePassword->getNewPassword());
    auto encryptPassphrase = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), this->m_retrievePassword->getPassphrase());

    auto reply = this->m_boxManagerProxy->RetrievePassword(this->m_uid,
                                                           encryptPassphrase,
                                                           encryptNewPassword);

    bool ret = false;
    reply.waitForFinished();
    if (reply.isValid())
    {
        ret = reply.value();
        if (!ret)
        {
            auto message = buildNotifyPage(QString(tr("Passphrase error!")));
            message->show();
        }
        else
        {
            auto message = buildNotifyPage(QString(tr("Retrieve success!")));
            message->show();
        }
    }
    else
    {
        KLOG_WARNING() << "Retrieve password failed. ret = " << ret;
    }
}

void Box::inputMountPasswordAccepted()
{
    auto encryptPasswd = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_inputMountPassword->getInputPassword());
    auto reply = this->m_boxManagerProxy->Mount(this->m_uid, encryptPasswd);
    bool ret = false;
    reply.waitForFinished();
    if (reply.isValid())
    {
        ret = reply.value();
        if (!ret)
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
}

void Box::inputDelBoxPasswordAccepted()
{
    auto encryptPasswd = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_inputDelBoxPassword->getInputPassword());
    auto reply = this->m_boxManagerProxy->DelBox(this->m_uid, encryptPasswd);
    bool ret = false;
    reply.waitForFinished();
    if (reply.isValid())
    {
        ret = reply.value();
        if (!ret)
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
}
}  // namespace KS
