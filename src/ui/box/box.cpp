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

#include "src/ui/box/box.h"
#include <QMenu>
#include <QMouseEvent>
#include "sc-i.h"
#include "src/ui/box/modify-password.h"
#include "src/ui/box_manager_proxy.h"

namespace KS
{
Box::Box(const QString &boxUID) : m_boxUID(boxUID),
                                  m_modifyPassword(nullptr),
                                  m_popupMenu(nullptr)
{
    this->m_boxManagerProxy = new BoxManagerProxy(SC_DBUS_NAME,
                                                  SC_BOX_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);

    this->setPixmap(QPixmap(":/box-big"));
    // this->setIcon(QIcon(":/box-big"));
    // this->setIconSize(QSize(64, 64));
    // this->setFlat(true);
    // this->setCheckable(false);

    auto mounted = this->m_boxManagerProxy->IsMounted(this->m_boxUID).value();

    // this->m_modifyPassword = new ModifyPassword();

    this->m_popupMenu = new QMenu(this);
    this->m_mountedStatusAction = this->m_popupMenu->addAction(mounted ? tr("Lock") : tr("Unlock"),
                                                               this,
                                                               &Box::switchMountedStatus);
    this->m_popupMenu->addAction(tr("Modify password"), this, &Box::modifyPassword);
    this->m_popupMenu->addAction(tr("Delete"));
}

void Box::boxChanged()
{
    // 保密箱属性发生变化，需要进行更新
    auto mounted = this->m_boxManagerProxy->IsMounted(this->m_boxUID).value();
    this->m_mountedStatusAction->setText(mounted ? tr("Lock") : tr("Unlock"));
}

void Box::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        this->m_popupMenu->popup(event->globalPos());
    }

    QLabel::mousePressEvent(event);
}

void Box::switchMountedStatus()
{
    auto mounted = this->m_boxManagerProxy->IsMounted(this->m_boxUID).value();

    if (mounted)
    {
        this->m_boxManagerProxy->UnMount(this->m_boxUID).waitForFinished();
    }
    else
    {
        this->m_boxManagerProxy->Mount(this->m_boxUID).waitForFinished();
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
    this->m_modifyPassword->show();
}

void Box::modifyPasswordAccepted()
{
    auto reply = this->m_boxManagerProxy->ModifyBoxPassword(this->m_boxUID,
                                                            this->m_modifyPassword->getCurrentPassword(),
                                                            this->m_modifyPassword->getNewPassword());

    reply.waitForFinished();
}
}  // namespace KS
