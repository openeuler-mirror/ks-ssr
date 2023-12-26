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
#include "ui_identity-authentication.h"
#include "src/ui/account_proxy.h"
#include "include/ssr-i.h"
#include "common/ssr-marcos-ui.h"

namespace KS
{
namespace Settings
{
IdentityAuthentication::IdentityAuthentication(QWidget *parent) :
    QWidget(parent),
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
    // TODO 多因子认证获取状态，设置m_twoFactor开关状态,连接m_twoFactor点击信号

    // TODO m_accountProxy获取状态，设置m_identification开关状态
    connect(m_ui->m_identification, &QCheckBox::clicked, this, [this](bool checked){
        auto reply = m_accountProxy->SetUidReusable(checked);
        CHECK_ERROR_FOR_DBUS_REPLY(reply);
    });
}
}  // namespace Settings
}  // namespace KS
