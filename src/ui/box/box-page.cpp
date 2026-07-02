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

#include "src/ui/box/box-page.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include "lib/base/crypto-helper.h"
#include "sc-i.h"
#include "src/ui/box/box.h"
#include "src/ui/box/create-box.h"
#include "src/ui/box/modify-password.h"
#include "src/ui/box/retrieve-password.h"
#include "src/ui/box_manager_proxy.h"
#include "src/ui/common/custom-window.h"
#include "src/ui/ui_box-page.h"

namespace KS
{
BoxPage::BoxPage() : QWidget(nullptr),
                     m_ui(new Ui::BoxPage()),
                     m_createBoxPage(nullptr),
                     m_createBox(nullptr),
                     m_modifyPasswordPage(nullptr),
                     m_retrievePasswordPage(nullptr),
                     m_passwdEdit(nullptr)
{
    this->m_ui->setupUi(this);

    this->m_boxManagerProxy = new BoxManagerProxy(SC_DBUS_NAME,
                                                  SC_BOX_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);

    this->m_ui->m_boxsScroll->setFrameStyle(QFrame::NoFrame);
    this->initBoxs();
    m_passwdEdit = new QLineEdit(this);
    m_passwdEdit->setFixedHeight(36);
    m_passwdEdit->setEchoMode(QLineEdit::Password);
    m_passwdEdit->hide();

    connect(this->m_boxManagerProxy, SIGNAL(BoxAdded(const QString &, const QString &)), this, SLOT(boxAdded(const QString &, const QString &)));
    connect(this->m_boxManagerProxy, SIGNAL(BoxDeleted(const QString &)), this, SLOT(boxDeleted(const QString &)));
    connect(this->m_boxManagerProxy, SIGNAL(BoxChanged(const QString &)), this, SLOT(boxChanged(const QString &)));
    connect(this->m_ui->m_newBox, SIGNAL(clicked(bool)), this, SLOT(newBoxClicked(bool)));
}

void BoxPage::initBoxs()
{
    QJsonParseError jsonError;
    auto reply = this->m_boxManagerProxy->GetBoxs();
    auto boxs = reply.value();

    auto jsonDoc = QJsonDocument::fromJson(boxs.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser boxs information failed: " << jsonError.errorString();
        return;
    }

    auto jsonRoot = jsonDoc.array();

    for (auto iter : jsonRoot)
    {
        auto jsonBox = iter.toObject();
        auto box = this->buildBox(jsonBox);
        this->addBox(box);
    }
}

Box *BoxPage::buildBox(const QJsonObject &jsonBox)
{
    auto boxUID = jsonBox.value(SCBM_JK_BOX_UID).toString();
    auto box = new Box(boxUID);
    return box;
}

void BoxPage::addBox(Box *box)
{
    this->m_ui->m_boxs->addBox(box);
    this->m_boxs.insert(box->getUID(), box);
    KLOG_DEBUG() << "insert box uid = " << box->getUID();
    box->disconnect();
    connect(box, &Box::unUnlockedIconClicked, this, [this]
            {
                auto notify = buildNotifyPage(tr("Box is locked, please unlocked!"));
                notify->show();
            });
    // 解锁
    connect(box, &Box::unlockedClicked, this, [this](const QString &boxUID)
            {
                auto inputPasswdPage = buildInputPasswdPage(boxUID, INPUT_PASSWORD_TYPE::MOUNT_PASSWORD);
                inputPasswdPage->show();
            });

    connect(this, &BoxPage::inputMountPasswdClicked, box, &Box::checkMountPasswd);

    connect(box, &Box::checkMountPasswdResult, this, [this](bool status)
            { this->inputPasswdNotify(tr("Unlock success!"),
                                      tr("Unlock failed, please check whether the password is correct."),
                                      status); });
    // 删除
    connect(box, &Box::delBoxClicked, this, [this](const QString &boxUID)
            {
                auto inputPasswdPage = buildInputPasswdPage(boxUID, INPUT_PASSWORD_TYPE::DELETE_BOX_PASSWORD);
                inputPasswdPage->show();
            });

    connect(this, &BoxPage::inputDelPasswdClicked, box, &Box::checkDelPasswd);

    connect(box, &Box::checkDelPasswdResult, this, [this](bool status)
            { this->inputPasswdNotify(tr("Delete success!"),
                                      tr("The Password is wrong or has been mounted!"),
                                      status); });
    // 修改密码
    connect(box, &Box::checkModifyPasswdResult, this, [this](bool status)
            {
                this->inputPasswdNotify(tr("Modify success!"),
                                        tr("Password error!"),
                                        status);
                this->m_modifyPasswordPage->close();
            });
    connect(box, &Box::modifyPasswordClicked, this, &BoxPage::showModifyPasswordPage);
    // 找回密码
    connect(box, &Box::checkRetrievePasswordResult, this, [this](bool status)
            {
                this->inputPasswdNotify(tr("Retrieve success!"),
                                        tr("Passphrase error!"),
                                        status);
                this->m_retrievePasswordPage->close();
            });
    connect(box, &Box::retrievePasswordClicked, this, &BoxPage::showRetrievePasswordPage);
}

void BoxPage::removeBox(const QString &boxUID)
{
    auto box = this->m_boxs.value(boxUID);
    if (box)
    {
        this->m_ui->m_boxs->removeBox(box);
        this->m_boxs.remove(box->getUID());
    }
}

TitlebarWindow *BoxPage::buildInputPasswdPage(const QString &boxUID, INPUT_PASSWORD_TYPE type)
{
    m_inputPasswdBoxUID = boxUID;
    auto *inputPasswdPage = new CustomWindow(this);
    inputPasswdPage->setTitle(tr("Input password"));
    inputPasswdPage->setFixedSize(300, 220);

    auto cusVLay = inputPasswdPage->getContentLayout();

    auto *label = new QLabel(tr("Please input password:"), inputPasswdPage);
    label->setFixedHeight(36);

    auto *hlay = new QHBoxLayout(inputPasswdPage);
    auto *ok = new QPushButton(tr("ok"), inputPasswdPage);
    ok->setFixedSize(72, 36);
    ok->setObjectName("passwdOkBtn");

    auto *cancel = new QPushButton(tr("cancel"), inputPasswdPage);
    cancel->setFixedSize(72, 36);
    cancel->setObjectName("passwdCancelBtn");

    if (type == INPUT_PASSWORD_TYPE::MOUNT_PASSWORD)
    {
        connect(ok, &QPushButton::clicked, this, [this]
                {
                    emit this->inputMountPasswdClicked(m_passwdEdit->text(), m_inputPasswdBoxUID);
                    this->m_passwdEdit->setText("");
                });
    }
    else if (type == INPUT_PASSWORD_TYPE::DELETE_BOX_PASSWORD)
    {
        connect(ok, &QPushButton::clicked, this, [this]
                {
                    emit this->inputDelPasswdClicked(m_passwdEdit->text(), m_inputPasswdBoxUID);
                    this->m_passwdEdit->setText("");
                });
    }
    connect(ok, &QPushButton::clicked, inputPasswdPage, &QWidget::close);
    connect(cancel, &QPushButton::clicked, inputPasswdPage, &QWidget::close);

    hlay->addWidget(ok);
    hlay->addWidget(cancel);

    cusVLay->addWidget(label);
    cusVLay->addWidget(m_passwdEdit);
    m_passwdEdit->show();
    cusVLay->addStretch();
    cusVLay->addLayout(hlay);

    int x = this->x() + this->width() / 4 + inputPasswdPage->width() / 4;
    int y = this->y() + this->height() / 4 + inputPasswdPage->height() / 4;
    inputPasswdPage->move(x, y);

    return inputPasswdPage;
}

TitlebarWindow *BoxPage::buildNotifyPage(const QString &notify)
{
    auto notifyPage = new CustomWindow(this);
    notifyPage->setFixedSize(240, 180);
    notifyPage->buildNotify(notify);

    int x = this->x() + this->width() / 4 + notifyPage->width() / 4;
    int y = this->y() + this->height() / 4 + notifyPage->height() / 4;
    notifyPage->move(x, y);

    return notifyPage;
}

void BoxPage::inputPasswdNotify(const QString &normal,
                                const QString &error,
                                bool status)
{
    if (status)
    {
        auto notify = buildNotifyPage(normal);
        notify->show();
    }
    else
    {
        auto notify = buildNotifyPage(error);
        notify->show();
    }
}

void BoxPage::boxAdded(const QString &boxUID, const QString &passphrase)
{
    QJsonParseError jsonError;

    auto reply = this->m_boxManagerProxy->GetBoxByUID(boxUID);
    reply.waitForFinished();

    // 可能无权限获取
    if (reply.isError())
    {
        KLOG_DEBUG() << "Cannot get box " << boxUID << " info: " << reply.error().message();
        return;
    }

    auto jsonStr = reply.value();
    auto jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser box information failed: " << jsonError.errorString();
        return;
    }

    auto jsonBox = jsonDoc.object();

    auto notify = this->buildNotifyPage(QString(tr("Please remember this box passphrase : %1")).arg(passphrase));
    notify->show();

    //    this->buildBox(jsonBox);
    //    this->addBox(box);
}

void BoxPage::boxDeleted(const QString &boxUID)
{
    this->removeBox(boxUID);
}

void BoxPage::boxChanged(const QString &boxUID)
{
    auto box = this->m_boxs.value(boxUID);
    box->boxChanged();
}

void BoxPage::newBoxClicked(bool checked)
{
    if (this->m_createBoxPage && this->m_createBox)
    {
        this->m_createBoxPage->show();
        this->m_createBox->show();
        return;
    }
    m_createBoxPage = new CustomWindow(this);

    m_createBoxPage->setTitle(tr("Add box"));
    m_createBoxPage->setFixedSize(400, 300);

    auto cusVLay = m_createBoxPage->getContentLayout();

    m_createBox = new CreateBox(m_createBoxPage);
    connect(m_createBox, &CreateBox::inputEmpty, this, [this]
            {
                auto notity = buildNotifyPage(tr("The input cannot be empty, please improve the information."));
                notity->show();
            });
    connect(m_createBox, &CreateBox::accepted, this, [this]
            {
                createBoxAccepted();
                m_createBoxPage->close();
            });
    connect(m_createBox, &CreateBox::rejected, this, [this]
            { m_createBoxPage->close(); });
    connect(m_createBox, &CreateBox::passwdInconsistent, this, [this]
            {
                auto notity = buildNotifyPage(tr("Please confirm whether the password is consistent."));
                notity->show();
            });

    cusVLay->addWidget(m_createBox);

    int x = this->x() + this->width() / 4 + m_createBoxPage->width() / 4;
    int y = this->y() + this->height() / 4 + m_createBoxPage->height() / 4;
    m_createBoxPage->move(x, y);
    m_createBoxPage->show();
    m_createBox->show();
}

void BoxPage::createBoxAccepted()
{
    // rsa加密
    auto encryptPassword = CryptoHelper::rsaEncrypt(this->m_boxManagerProxy->rSAPublicKey(), this->m_createBox->getPassword());
    auto reply = this->m_boxManagerProxy->CreateBox(this->m_createBox->getName(),
                                                    encryptPassword);

    auto boxID = reply.value();
    auto box = new Box(boxID);
    this->addBox(box);
}

void BoxPage::showModifyPasswordPage(ModifyPassword *modifyPassword)
{
    m_modifyPasswordPage = new CustomWindow(this);
    m_modifyPasswordPage->setTitle(tr("Modify password"));
    m_modifyPasswordPage->setFixedSize(400, 320);

    auto cusVLay = m_modifyPasswordPage->getContentLayout();
    cusVLay->addWidget(modifyPassword);

    connect(modifyPassword, &ModifyPassword::inputEmpty, this, [this]
            {
                auto notity = buildNotifyPage(tr("The input cannot be empty, please improve the information."));
                notity->show();
            });
    connect(modifyPassword, &ModifyPassword::passwdInconsistent, this, [this]
            {
                auto notity = buildNotifyPage(tr("Please confirm whether the password is consistent."));
                notity->show();
            });
    connect(modifyPassword, &ModifyPassword::rejected, this, [this]
            { this->m_modifyPasswordPage->close(); });

    modifyPassword->show();
    int x = this->x() + this->width() / 4 + m_modifyPasswordPage->width() / 4;
    int y = this->y() + this->height() / 4 + m_modifyPasswordPage->height() / 4;
    m_modifyPasswordPage->move(x, y);
    m_modifyPasswordPage->show();
}

void BoxPage::showRetrievePasswordPage(RetrievePassword *retrievePassword)
{
    m_retrievePasswordPage = new CustomWindow(this);
    m_retrievePasswordPage->setTitle(tr("Retrieve password"));
    m_retrievePasswordPage->setFixedSize(400, 320);

    auto cusVLay = m_retrievePasswordPage->getContentLayout();
    cusVLay->addWidget(retrievePassword);

    connect(retrievePassword, &RetrievePassword::inputEmpty, this, [this]
            {
                auto notity = buildNotifyPage(tr("The input cannot be empty, please improve the information."));
                notity->show();
            });
    connect(retrievePassword, &RetrievePassword::passwdInconsistent, this, [this]
            {
                auto notity = buildNotifyPage(tr("Please confirm whether the password is consistent."));
                notity->show();
            });
    connect(retrievePassword, &RetrievePassword::rejected, this, [this]
            { this->m_retrievePasswordPage->close(); });

    retrievePassword->show();
    int x = this->x() + this->width() / 4 + m_retrievePasswordPage->width() / 4;
    int y = this->y() + this->height() / 4 + m_retrievePasswordPage->height() / 4;
    m_retrievePasswordPage->move(x, y);
    m_retrievePasswordPage->show();
}
}  // namespace KS
