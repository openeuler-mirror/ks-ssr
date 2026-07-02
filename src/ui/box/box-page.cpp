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

#include "src/ui/box/box-page.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include "ksc-i.h"
#include "lib/base/crypto-helper.h"
#include "src/ui/box/box.h"
#include "src/ui/box/create-box.h"
#include "src/ui/box_manager_proxy.h"
#include "src/ui/common/sub-window.h"
#include "src/ui/ui_box-page.h"

namespace KS
{
BoxPage::BoxPage() : QWidget(nullptr),
                     m_ui(new Ui::BoxPage()),
                     m_createBox(nullptr)
{
    this->m_ui->setupUi(this);

    this->m_boxManagerProxy = new BoxManagerProxy(KSC_DBUS_NAME,
                                                  KSC_BOX_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);

    this->m_ui->m_boxsScroll->setFrameStyle(QFrame::NoFrame);

    initBoxs();

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
    auto boxUID = jsonBox.value(KSC_BM_JK_BOX_UID).toString();
    auto box = new Box(boxUID);
    return box;
}

void BoxPage::addBox(Box *box)
{
    this->m_ui->m_boxs->addBox(box);
    this->m_boxs.insert(box->getUID(), box);
    KLOG_DEBUG() << "insert box uid = " << box->getUID();
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

    auto messge = new SubWindow(this);
    messge->buildNotify(QString(tr("Please remember this box passphrase : %1")).arg(passphrase));
    messge->setFixedSize(240, 180);
    int x = this->window()->x() + this->window()->width() / 4 + messge->width() / 4;
    int y = this->window()->y() + this->window()->height() / 4 + messge->height() / 4;
    messge->move(x, y);
    messge->show();

    //    auto box = this->buildBox(jsonBox);
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
    auto createBox = new SubWindow(this);
    createBox->setFixedSize(400, 300);
    createBox->setTitle(tr("Create box"));

    this->m_createBox = new CreateBox(createBox);
    connect(this->m_createBox, SIGNAL(accepted()), this, SLOT(createBoxAccepted()));
    connect(this->m_createBox, &CreateBox::passwdInconsistent, this, [this]
            {
                auto messge = new SubWindow(this);
                messge->buildNotify(tr("Please confirm whether the password is consistent."));
                messge->setFixedSize(240, 180);
                int x = this->window()->x() + this->window()->width() / 4 + messge->width() / 4;
                int y = this->window()->y() + this->window()->height() / 4 + messge->height() / 4;
                messge->move(x, y);
                messge->show();
            });
    connect(this->m_createBox, &CreateBox::inputEmpty, this, [this]
            {
                auto messge = new SubWindow(this);
                messge->buildNotify(QString(tr("The input cannot be empty, please improve the information.")));
                messge->setFixedSize(240, 180);
                int x = this->window()->x() + this->window()->width() / 4 + messge->width() / 4;
                int y = this->window()->y() + this->window()->height() / 4 + messge->height() / 4;
                messge->move(x, y);
                messge->show();
            });

    connect(this->m_createBox, &CreateBox::accepted, createBox, &SubWindow::close);
    connect(this->m_createBox, &CreateBox::rejected, createBox, &SubWindow::close);

    //    this->m_modifyPassword->show();

    createBox->getContentLayout()->addWidget(m_createBox);

    int x = this->window()->x() + this->window()->width() / 4 + createBox->width() / 4;
    int y = this->window()->y() + this->window()->height() / 4 + createBox->height() / 4;
    createBox->move(x, y);
    createBox->show();
}

void BoxPage::createBoxAccepted()
{
    // rsa加密
    auto encryptPassword = CryptoHelper::rsaEncrypt(this->m_boxManagerProxy->rSAPublicKey(), this->m_createBox->getPassword());
    auto reply = this->m_boxManagerProxy->CreateBox(this->m_createBox->getName(),
                                                    encryptPassword);

    auto boxID = reply.value();
    auto box = new Box(boxID);
    //    this->m_ui->m_boxs->addBox(box);
    this->addBox(box);
}
}  // namespace KS
