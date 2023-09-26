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
#include "src/ui/common/message-dialog.h"
#include "src/ui/ui_box-page.h"

namespace KS
{
BoxPage::BoxPage() : QWidget(nullptr),
                     m_ui(new Ui::BoxPage()),
                     m_createBox(nullptr)
{
    m_ui->setupUi(this);

    m_boxManagerProxy = new BoxManagerProxy(KSC_DBUS_NAME,
                                            KSC_BOX_MANAGER_DBUS_OBJECT_PATH,
                                            QDBusConnection::systemBus(),
                                            this);

    m_ui->m_boxsScroll->setFrameStyle(QFrame::NoFrame);

    initBoxs();

    connect(m_boxManagerProxy, SIGNAL(BoxAdded(const QString &)), this, SLOT(boxAdded(const QString &)));
    connect(m_boxManagerProxy, SIGNAL(BoxDeleted(const QString &)), this, SLOT(boxDeleted(const QString &)));
    connect(m_boxManagerProxy, SIGNAL(BoxChanged(const QString &)), this, SLOT(boxChanged(const QString &)));
    connect(m_ui->m_newBox, SIGNAL(clicked(bool)), this, SLOT(newBoxClicked(bool)));
}

void BoxPage::initBoxs()
{
    QJsonParseError jsonError;
    auto reply = m_boxManagerProxy->GetBoxs();
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
        auto box = buildBox(jsonBox);
        addBox(box);
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
    m_ui->m_boxs->addBox(box);
    m_boxs.insert(box->getUID(), box);
    KLOG_DEBUG() << "insert box uid = " << box->getUID();
}

void BoxPage::removeBox(const QString &boxUID)
{
    auto box = m_boxs.value(boxUID);
    if (box)
    {
        m_ui->m_boxs->removeBox(box);
        m_boxs.remove(box->getUID());
    }
}

void BoxPage::boxAdded(const QString &boxUID)
{
    QJsonParseError jsonError;

    auto reply = m_boxManagerProxy->GetBoxByUID(boxUID);
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

    //    auto box = buildBox(jsonBox);
    //    addBox(box);
}

void BoxPage::boxDeleted(const QString &boxUID)
{
    removeBox(boxUID);
}

void BoxPage::boxChanged(const QString &boxUID)
{
    auto box = m_boxs.value(boxUID);
    box->boxChanged();
}

void BoxPage::newBoxClicked(bool checked)
{
    m_createBox = new CreateBox(this);
    m_createBox->setFixedSize(400, 350);
    m_createBox->setTitle(tr("Create box"));

    connect(m_createBox, SIGNAL(accepted()), this, SLOT(createBoxAccepted()));
    connect(m_createBox, &CreateBox::passwdInconsistent, this, [this]
            {
                auto messgeDialog = new MessageDialog(this);
                messgeDialog->setMessage(tr("Please confirm whether the password is consistent."));
                int x = window()->x() + window()->width() / 4 + messgeDialog->width() / 4;
                int y = window()->y() + window()->height() / 4 + messgeDialog->height() / 4;
                messgeDialog->move(x, y);
                messgeDialog->show();
            });
    connect(m_createBox, &CreateBox::inputEmpty, this, [this]
            {
                auto messgeDialog = new MessageDialog(this);
                messgeDialog->setMessage(QString(tr("The input cannot be empty, please improve the information.")));
                int x = window()->x() + window()->width() / 4 + messgeDialog->width() / 4;
                int y = window()->y() + window()->height() / 4 + messgeDialog->height() / 4;
                messgeDialog->move(x, y);
                messgeDialog->show();
            });

    int x = window()->x() + window()->width() / 4 + m_createBox->width() / 4;
    int y = window()->y() + window()->height() / 4 + m_createBox->height() / 8;
    m_createBox->move(x, y);
    m_createBox->show();
}

void BoxPage::createBoxAccepted()
{
    // 口令
    QString passphrase;
    // rsa加密
    auto encryptPassword = CryptoHelper::rsaEncrypt(m_boxManagerProxy->rSAPublicKey(), m_createBox->getPassword());
    auto reply = m_boxManagerProxy->CreateBox(m_createBox->getName(),
                                              encryptPassword,
                                              passphrase);
    auto boxID = reply.value();
    if (!reply.error().message().isEmpty())
    {
        auto messgeDialog = new MessageDialog(this);
        messgeDialog->setMessage(reply.error().message());
        int x = window()->x() + window()->width() / 4 + messgeDialog->width() / 4;
        int y = window()->y() + window()->height() / 4 + messgeDialog->height() / 4;
        messgeDialog->move(x, y);
        messgeDialog->show();
        return;
    }

    auto box = new Box(boxID);
    //    m_ui->m_boxs->addBox(box);
    addBox(box);
    // 显示消息
    auto messgeDialog = new MessageDialog(this);
    messgeDialog->setMessage(QString(tr("Please remember this box passphrase : %1, Can be used to retrieve passwords.")).arg(passphrase));
    messgeDialog->setFixedSize(240, 200);
    int x = window()->x() + window()->width() / 4 + messgeDialog->width() / 4;
    int y = window()->y() + window()->height() / 4 + messgeDialog->height() / 4;
    messgeDialog->move(x, y);
    messgeDialog->show();
}
}  // namespace KS
