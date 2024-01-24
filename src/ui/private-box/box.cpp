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

#include "src/ui/private-box/box.h"
#include <qt5-log-i.h>
#include <QDesktopServices>
#include <QFontMetrics>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include "lib/base/crypto-helper.h"
#include "src/ui/box_manager_proxy.h"
#include "src/ui/common/password-modification.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/common/user-prompt-dialog.h"
#include "src/ui/private-box/box-password-checked.h"
#include "src/ui/private-box/box-password-retrieve.h"
#include "ssr-i.h"

namespace KS
{
namespace PrivateBox
{
Box::Box(const QString &uid)
    : m_uid(uid),
      m_name("Unknown"),
      m_mounted(false),
      m_modifyPassword(nullptr),
      m_retrievePassword(nullptr),
      m_popupMenu(nullptr)
{
    m_boxManagerProxy = new BoxManagerProxy(SSR_DBUS_NAME,
                                            SSR_BOX_MANAGER_DBUS_OBJECT_PATH,
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
    m_showingName->setAlignment(Qt::AlignCenter);
    m_showingName->setMaximumWidth(102);
    // 过长使用省略号显示
    QFontMetrics fontMetrics(this->fontMetrics());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    auto fontSize = fontMetrics.horizontalAdvance(m_name);
#else
    auto fontSize = fontMetrics.width(m_name);
#endif

    QString str = m_name;
    if (fontSize > m_showingName->width())
    {
        str = fontMetrics.elidedText(m_name, Qt::ElideRight, m_showingName->width());
        m_showingName->setToolTip(m_name);
    }

    m_showingName->setText(str);

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
    m_name = jsonBox.value(SSR_BM_JK_BOX_NAME).toString();
    m_mounted = jsonBox.value(SSR_BM_JK_BOX_MOUNTED).toBool();
}

void Box::initMenu()
{
    auto mounted = m_boxManagerProxy->IsMounted(m_uid).value();
    m_popupMenu = new QMenu(this);
    m_popupMenu->setObjectName("m_popupMenu");

    m_mountedStatusAction = m_popupMenu->addAction(mounted ? tr("Lock") : tr("Unlock"),
                                                   this,
                                                   &Box::switchMountedStatus);
    m_popupMenu->addAction(tr("Modify password"), this, &Box::modifyPassword);
    m_popupMenu->addAction(tr("Delete"), this, &Box::popDeleteNotify);
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
        auto reply = m_boxManagerProxy->UnMount(m_uid);
        CHECK_ERROR_FOR_DBUS_REPLY(reply);
        return;
    }
    else
    {
        m_inputMountPassword = new BoxPasswordChecked(window());
        m_inputMountPassword->setTitle(tr("Unlock"));
        connect(m_inputMountPassword, &BoxPasswordChecked::accepted, this, &Box::acceptedInputMountPassword);

        auto x = window()->x() + window()->width() / 2 - m_inputMountPassword->width() / 2;
        auto y = window()->y() + window()->height() / 2 - m_inputMountPassword->height() / 2;
        m_inputMountPassword->move(x, y);
        m_inputMountPassword->show();
    }
}

void Box::modifyPassword()
{
    m_modifyPassword = new PasswordModification(window());
    connect(m_modifyPassword, SIGNAL(accepted()), this, SLOT(acceptedModifyPassword()));

    m_modifyPassword->setTitleNameTail(m_name);

    auto x = window()->x() + window()->width() / 2 - m_modifyPassword->width() / 2;
    auto y = window()->y() + window()->height() / 2 - m_modifyPassword->height() / 2;
    m_modifyPassword->move(x, y);
    m_modifyPassword->show();
}

void Box::popDeleteNotify()
{
    auto deleteNotify = new UserPromptDialog(this);
    deleteNotify->setNotifyMessage(tr("Remove box"), tr("The operation will delete the content inside the box."
                                                        "Are you sure you want to delete it?"));
    auto x = window()->x() + window()->width() / 2 - deleteNotify->width() / 2;
    auto y = window()->y() + window()->height() / 2 - deleteNotify->height() / 2;

    deleteNotify->move(x, y);
    deleteNotify->show();

    connect(deleteNotify, &UserPromptDialog::accepted, this, &Box::delBox);
}

void Box::delBox()
{
    m_inputDelBoxPassword = new BoxPasswordChecked(window());
    m_inputDelBoxPassword->setTitle(tr("Del box"));
    connect(m_inputDelBoxPassword, &BoxPasswordChecked::accepted, this, &Box::acceptedInputDelBoxPassword);

    auto x = window()->x() + window()->width() / 2 - m_inputDelBoxPassword->width() / 2;
    auto y = window()->y() + window()->height() / 2 - m_inputDelBoxPassword->height() / 2;
    m_inputDelBoxPassword->move(x, y);
    m_inputDelBoxPassword->show();
}

void Box::retrievePassword()
{
    m_retrievePassword = new BoxPasswordRetrieve(window());
    m_retrievePassword->setFixedSize(319, 239);
    m_retrievePassword->setTitle(tr("Retrieve password"));
    connect(m_retrievePassword, SIGNAL(accepted()), this, SLOT(acceptedRetrievePassword()));
    connect(m_retrievePassword, &BoxPasswordRetrieve::inputEmpty, this, [this] {
        POPUP_MESSAGE_DIALOG(tr("The input cannot be empty, please improve the information."));
    });

    auto x = window()->x() + window()->width() / 2 - m_retrievePassword->width() / 2;
    auto y = window()->y() + window()->height() / 2 - m_retrievePassword->height() / 2;
    m_retrievePassword->move(x, y);
    m_retrievePassword->show();
}

void Box::acceptedModifyPassword()
{
    auto encryptCurrentPassword = CryptoHelper::rsaEncryptString(m_boxManagerProxy->rSAPublicKey(), m_modifyPassword->getCurrentPassword());
    auto encryptNewPassword = CryptoHelper::rsaEncryptString(m_boxManagerProxy->rSAPublicKey(), m_modifyPassword->getNewPassword());
    auto reply = m_boxManagerProxy->ModifyBoxPassword(m_uid,
                                                      encryptCurrentPassword,
                                                      encryptNewPassword);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    if (!reply.isError())
    {
        POPUP_MESSAGE_DIALOG(tr("Modify success!"));
    }
}

void Box::acceptedRetrievePassword()
{
    auto encryptPassphrase = CryptoHelper::rsaEncryptString(m_boxManagerProxy->rSAPublicKey(), m_retrievePassword->getPassphrase());
    auto reply = m_boxManagerProxy->RetrieveBoxPassword(m_uid, encryptPassphrase);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    if (!reply.isError())
    {
        POPUP_MESSAGE_DIALOG(QString(tr("Your box password is %1")).arg(reply.value()));
    }
}

void Box::acceptedInputMountPassword()
{
    auto encryptPasswd = CryptoHelper::rsaEncryptString(m_boxManagerProxy->rSAPublicKey(), m_inputMountPassword->getBoxPasswordChecked());
    auto reply = m_boxManagerProxy->Mount(m_uid, encryptPasswd);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    if (!reply.isError())
    {
        POPUP_MESSAGE_DIALOG(QString(tr("Unlock success!")));
    }
}

void Box::acceptedInputDelBoxPassword()
{
    auto encryptPasswd = CryptoHelper::rsaEncryptString(m_boxManagerProxy->rSAPublicKey(), m_inputDelBoxPassword->getBoxPasswordChecked());
    auto reply = m_boxManagerProxy->DelBox(m_uid, encryptPasswd);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    if (!reply.isError())
    {
        POPUP_MESSAGE_DIALOG(QString(tr("Delete success!")));
    }
}
}  // namespace PrivateBox
}  // namespace KS
