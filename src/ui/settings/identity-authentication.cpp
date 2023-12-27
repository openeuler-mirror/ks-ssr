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
                auto reply = m_accountProxy->SetMultiFactorAuthState(checked);
                CHECK_ERROR_FOR_DBUS_REPLY(reply);
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
