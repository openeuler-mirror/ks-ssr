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
#include "lib/base/crypto-helper.h"
#include "sc-i.h"
#include "src/ui/box/modify-password.h"
#include "src/ui/box_manager_proxy.h"

namespace KS
{
Box::Box(const QString &uid) : m_uid(uid),
                               m_name("Unknown"),
                               m_mounted(false),
                               m_modifyPassword(nullptr),
                               m_popupMenu(nullptr)
{
    this->m_boxManagerProxy = new BoxManagerProxy(SC_DBUS_NAME,
                                                  SC_BOX_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);

    m_passwdEdit = new QLineEdit;
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
    this->m_name = jsonBox.value(SCBM_JK_BOX_NAME).toString();
    this->m_mounted = QVariant(jsonBox.value(SCBM_JK_BOX_MOUNTED).toString()).toBool();
}

void Box::initMenu()
{
    auto mounted = this->m_boxManagerProxy->IsMounted(this->m_uid).value();

    // this->m_modifyPassword = new ModifyPassword();

    this->m_popupMenu = new QMenu(this);
    this->m_mountedStatusAction = this->m_popupMenu->addAction(mounted ? tr("Lock") : tr("Unlock"),
                                                               this,
                                                               &Box::switchMountedStatus);
    this->m_popupMenu->addAction(tr("Modify password"), this, &Box::modifyPassword);
    this->m_popupMenu->addAction(tr("Delete"), this, &Box::delBox);
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
        QWidget *widget = buildNotifyPage(tr("Box is locked, please unlocked!"));
        widget->show();
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
        QWidget *widget = buildMountInputPasswdPage();
        int x = this->x() + this->width() / 2 + widget->width() / 2;
        int y = this->y() + this->height() / 2 + widget->height() / 2;
        widget->move(x, y);
        widget->show();
    }
}

void Box::modifyPassword()
{
    if (this->m_modifyPassword)
    {
        this->m_modifyPassword->show();
        return;
    }

    this->m_modifyPassword = new ModifyPassword();
    connect(this->m_modifyPassword, SIGNAL(accepted()), this, SLOT(modifyPasswordAccepted()));

    int x = this->x() + this->width() / 4 + m_modifyPassword->width() / 4;
    int y = this->y() + this->height() / 4 + m_modifyPassword->height() / 4;
    this->m_modifyPassword->move(x, y);
    this->m_modifyPassword->setBoxName(this->m_name);
    this->m_modifyPassword->show();
}

void Box::delBox()
{
    QWidget *widget = new QWidget;
    widget->setWindowTitle(tr("input password"));
    widget->setWindowModality(Qt::ApplicationModal);
    widget->setWindowIcon(QIcon(":/images/logo"));

    QVBoxLayout *vlay = new QVBoxLayout(widget);

    QLabel *label = new QLabel(tr("Please input password:"));
    //     m_passwdEdit = new QLineEdit(widget);

    QHBoxLayout *hlay = new QHBoxLayout;
    QPushButton *ok = new QPushButton(tr("ok"));
    QPushButton *canel = new QPushButton(tr("canel"));
    connect(ok, &QPushButton::clicked, this, [this]
            {
                auto encryptPasswd = CryptoHelper::rsa_encrypt(m_boxManagerProxy->rSAPublicKey(), m_passwdEdit->text());
                auto reply = this->m_boxManagerProxy->DelBox(this->m_uid, encryptPasswd);
                bool ret = false;
                reply.waitForFinished();
                if (reply.isValid())
                {
                    ret = reply.value();
                    if (!ret)
                    {
                        QWidget *widget = buildNotifyPage(QString(tr("The Password is wrong or has been mounted!")));
                        widget->show();
                    }
                    else
                    {
                        QWidget *widget = buildNotifyPage(QString(tr("Delete success!")));
                        widget->show();
                    }
                }
                this->m_passwdEdit->setText("");
            });
    connect(ok, &QPushButton::clicked, widget, &QWidget::close);
    connect(canel, &QPushButton::clicked, widget, &QWidget::close);

    hlay->addWidget(ok);
    hlay->addWidget(canel);

    vlay->addWidget(label);
    vlay->addWidget(m_passwdEdit);
    vlay->addLayout(hlay);

    int x = this->x() + this->width() / 4 + widget->width() / 4;
    int y = this->y() + this->height() / 4 + widget->height() / 4;
    widget->move(x, y);
    widget->show();
}

QWidget *Box::buildMountInputPasswdPage()
{
    QWidget *widget = new QWidget;
    widget->setWindowTitle(tr("input password"));
    widget->setWindowModality(Qt::ApplicationModal);
    widget->setWindowIcon(QIcon(":/images/logo"));

    QVBoxLayout *vlay = new QVBoxLayout(widget);

    QLabel *label = new QLabel(tr("Please input password:"));
    //    m_passwdEdit = new QLineEdit(widget);

    QHBoxLayout *hlay = new QHBoxLayout;
    QPushButton *ok = new QPushButton(tr("ok"));
    QPushButton *canel = new QPushButton(tr("canel"));
    connect(ok, &QPushButton::clicked, this, [this]
            {
                auto encryptPasswd = CryptoHelper::rsa_encrypt(m_boxManagerProxy->rSAPublicKey(), m_passwdEdit->text());
                auto reply = this->m_boxManagerProxy->Mount(this->m_uid, encryptPasswd);
                bool ret = false;
                reply.waitForFinished();
                if (reply.isValid())
                {
                    ret = reply.value();
                    if (!ret)
                    {
                        QWidget *widget = buildNotifyPage(QString(tr("Password error!")));
                        widget->show();
                    }
                    else
                    {
                        QWidget *widget = buildNotifyPage(QString(tr("Unlock success!")));
                        widget->show();
                    }
                }
                this->m_passwdEdit->setText("");
            });
    connect(ok, &QPushButton::clicked, widget, &QWidget::close);
    connect(canel, &QPushButton::clicked, widget, &QWidget::close);

    hlay->addWidget(ok);
    hlay->addWidget(canel);

    vlay->addWidget(label);
    vlay->addWidget(m_passwdEdit);
    vlay->addLayout(hlay);

    widget->hide();

    return widget;
}

QWidget *Box::buildNotifyPage(const QString &notify)
{
    QWidget *widget = new QWidget();
    widget->setWindowModality(Qt::ApplicationModal);
    widget->setWindowTitle(tr("notice"));
    widget->setWindowIcon(QIcon(":/images/logo"));
    QVBoxLayout *vlay = new QVBoxLayout(widget);

    QLabel *label = new QLabel(notify);
    QPushButton *ok = new QPushButton(tr("ok"));
    connect(ok, &QPushButton::clicked, widget, &QWidget::close);

    vlay->addWidget(label);
    vlay->addWidget(ok);

    int x = this->x() + this->width() / 4 + widget->width() / 4;
    int y = this->y() + this->height() / 4 + widget->height() / 4;
    widget->move(x, y);

    return widget;
}

void Box::modifyPasswordAccepted()
{
    auto encryptCurrentPassword = CryptoHelper::rsa_encrypt(m_boxManagerProxy->rSAPublicKey(), this->m_modifyPassword->getCurrentPassword());
    auto encryptNewPassword = CryptoHelper::rsa_encrypt(m_boxManagerProxy->rSAPublicKey(), this->m_modifyPassword->getNewPassword());

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
            QWidget *widget = buildNotifyPage(QString(tr("Password error!")));
            widget->show();
        }
        else
        {
            QWidget *widget = buildNotifyPage(QString(tr("Modify success!")));
            widget->show();
        }
    }
    else
    {
        KLOG_DEBUG() << "modify password failed. ret = " << ret;
    }
}
}  // namespace KS
