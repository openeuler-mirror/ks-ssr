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

#include "identity-authentication.h"
#include "common/ssr-marcos-ui.h"
#include "common/user-prompt-dialog.h"
#include "include/ssr-i.h"
#include "src/ui/account_proxy.h"
#include "ui_identity-authentication.h"

namespace KS
{
namespace Settings
{
IdentityAuthentication::IdentityAuthentication(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::IdentityAuthentication)
{
    m_ui->setupUi(this);
    m_accountProxy = new AccountProxy(SSR_DBUS_NAME,
                                      SSR_ACCOUNT_DBUS_OBJECT_PATH,
                                      QDBusConnection::systemBus(),
                                      this);
    initConnection();
}

IdentityAuthentication::~IdentityAuthentication()
{
    delete m_ui;
}

void IdentityAuthentication::initConnection()
{
    m_ui->m_twoFactor->setCheckState(m_accountProxy->GetMultiFactorAuthState() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    connect(m_ui->m_twoFactor, &QCheckBox::clicked, this, [this](bool checked)
            {
                // 开启时二次确认，关闭时不提示
                if (checked)
                {
                    auto notify = new UserPromptDialog(this);
                    notify->setNotifyMessage(tr("Two-factor authentication"), tr("After turning on the switch, please make sure to enable the device in the peripheral management and"
                                                                                 " bind the UKEY device in the control center before restarting the host to make the function effective."));
                    notify->setFixedSize(320, 280);
                    auto x = window()->x() + window()->width() / 2 - notify->width() / 2;
                    auto y = window()->y() + window()->height() / 2 - notify->height() / 2;
                    notify->move(x, y);
                    notify->show();
                    connect(notify, &UserPromptDialog::rejected, this, [this]
                            {
                                m_ui->m_twoFactor->setChecked(false);
                            });
                    connect(notify, &UserPromptDialog::accepted, this, [this]
                            {
                                auto reply = m_accountProxy->SetMultiFactorAuthState(m_ui->m_twoFactor->isChecked());
                                CHECK_ERROR_FOR_DBUS_REPLY(reply);
                            });
                }
                else
                {
                    auto reply = m_accountProxy->SetMultiFactorAuthState(checked);
                    CHECK_ERROR_FOR_DBUS_REPLY(reply);
                }
            });
    m_ui->m_identification->setCheckState(m_accountProxy->GetUidReusable() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    connect(m_ui->m_identification, &QCheckBox::clicked, this, [this](bool checked)
            {
                auto reply = m_accountProxy->SetUidReusable(checked);
                CHECK_ERROR_FOR_DBUS_REPLY(reply);
            });
}
}  // namespace Settings
}  // namespace KS
