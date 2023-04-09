/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * kiran-session-manager is licensed under Mulan PSL v2.
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

#include "src/ui/box/box-manager.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>
#include "sc-i.h"
#include "src/ui/box/box.h"
#include "src/ui/box/create-box.h"
#include "src/ui/box_manager_proxy.h"
#include "src/ui/ui_box-manager.h"

namespace KS
{
BoxManager::BoxManager() : QWidget(nullptr),
                           m_ui(new Ui::BoxManager()),
                           m_createBox(nullptr)
{
    this->m_ui->setupUi(this);

    this->m_boxManagerProxy = new BoxManagerProxy(SC_DBUS_NAME,
                                                  SC_BOX_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);

    this->m_ui->m_boxsScroll->setFrameStyle(QFrame::NoFrame);

    initBoxs();

    connect(this->m_boxManagerProxy, SIGNAL(BoxAdded(const QString &)), this, SLOT(boxAdded(const QString &)));
    connect(this->m_boxManagerProxy, SIGNAL(BoxDeleted(const QString &)), this, SLOT(boxDeleted(const QString &)));
    connect(this->m_boxManagerProxy, SIGNAL(BoxChanged(const QString &)), this, SLOT(boxChanged(const QString &)));
    connect(this->m_ui->m_newBox, SIGNAL(clicked(bool)), this, SLOT(newBoxClicked(bool)));
}

void BoxManager::initBoxs()
{
    // TODO: test
    for (int i = 0; i <= 50; ++i)
    {
        auto box = new Box(QString("ID%1").arg(i));
        this->m_ui->m_boxs->addBox(box);
    }

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
    }
}

Box *BoxManager::buildBox(const QJsonObject &jsonBox)
{
    auto boxUID = jsonBox.value(SCBM_JK_BOX_UID).toString();
    auto box = new Box(boxUID);
    return box;
}

void BoxManager::addBox(Box *box)
{
    this->m_ui->m_boxs->addBox(box);
    this->m_boxs.insert(box->getUID(), box);
}

void BoxManager::removeBox(const QString &boxUID)
{
    auto box = this->m_boxs.value(boxUID);
    if (box)
    {
        this->m_ui->m_boxs->removeBox(box);
        this->m_boxs.remove(box->getUID());
    }
}

void BoxManager::boxAdded(const QString &boxUID)
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
    auto box = this->buildBox(jsonBox);
    this->addBox(box);
}

void BoxManager::boxDeleted(const QString &boxUID)
{
    this->removeBox(boxUID);
}

void BoxManager::boxChanged(const QString &boxUID)
{
    auto box = this->m_boxs.value(boxUID);
    if (box)
    {
        box->boxChanged();
    }
}

void BoxManager::newBoxClicked(bool checked)
{
    if (this->m_createBox)
    {
        this->m_createBox->show();
        return;
    }

    this->m_createBox = new CreateBox();
    connect(this->m_createBox, SIGNAL(accepted()), this, SLOT(createBoxAccepted()));

    this->m_createBox->show();
}

void BoxManager::createBoxAccepted()
{
    // TODO: 换成dbus
    if (0)
    {
        auto reply = this->m_boxManagerProxy->CreateBox(this->m_createBox->getName(),
                                                        this->m_createBox->getPassword());
        auto boxID = reply.value();
        auto box = new Box(boxID);
        this->m_ui->m_boxs->addBox(box);
    }
}
}  // namespace KS
