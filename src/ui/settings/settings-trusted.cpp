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
#include "settings-trusted.h"
#include "include/ksc-i.h"
#include "src/ui/common/ksc-marcos-ui.h"
#include "src/ui/kss_dbus_proxy.h"
#include "src/ui/settings/trusted-user-pin.h"
#include "ui_settings-trusted.h"
namespace KS
{
SettingsTrusted::SettingsTrusted(QWidget *parent) : QWidget(parent),
                                                    m_ui(new Ui::SettingsTrusted)
{
    m_ui->setupUi(this);

    m_kssDbusProxy = new KSSDbusProxy(KSC_DBUS_NAME,
                                      KSC_KSS_INIT_DBUS_OBJECT_PATH,
                                      QDBusConnection::systemBus(),
                                      this);

    initUI();
}

SettingsTrusted::~SettingsTrusted()
{
    delete m_ui;
}

void SettingsTrusted::initUI()
{
    // switch
    m_ui->m_switch->setCheckable(true);
    m_ui->m_switch->setChecked(m_kssDbusProxy->trustedStatus());
    m_ui->m_switch->setFixedSize(52, 24);
    m_ui->m_switch->setIconSize(QSize(52, 24));
    connect(m_ui->m_switch, &QPushButton::clicked, this, &SettingsTrusted::setTrustedStatus);

    // radio
    m_ui->m_soft->setCheckable(true);
    m_ui->m_soft->setIconSize(QSize(14, 14));

    m_ui->m_hard->setCheckable(true);
    m_ui->m_hard->setIconSize(QSize(14, 14));
    m_ui->m_hardLabel->setWordWrap(true);

    updateStorageMode();

    connect(m_ui->m_soft, &QPushButton::clicked, this, &SettingsTrusted::updateSoftRadio);
    connect(m_ui->m_hard, &QPushButton::clicked, this, &SettingsTrusted::updateHardRadio);
}

void SettingsTrusted::updateStorageMode()
{
    // 通过后台是否设置成功修改按钮状态
    auto mode = m_kssDbusProxy->storageMode();
    m_ui->m_soft->setChecked(mode == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? true : false);
    m_ui->m_hard->setChecked(mode == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? false : true);
}

bool SettingsTrusted::checkTrustedLoadFinied()
{
    // 可信未初始化完成，不允许操作
    if (!m_kssDbusProxy->initialized())
    {
        POPUP_MESSAGE_DIALOG(tr("Trusted data needs to be initialised,"
                                "please wait a few minutes before trying."));
        return false;
    }
    return true;
}

void SettingsTrusted::setTrustedStatus(bool checked)
{
    if (!checkTrustedLoadFinied())
    {
        m_ui->m_switch->setChecked(!checked);
        return;
    }
    auto reply = m_kssDbusProxy->SetTrustedStatus(checked);
    reply.waitForFinished();

    if (reply.isError())
    {
        POPUP_MESSAGE_DIALOG(reply.error().message());
        m_ui->m_switch->setChecked(!checked);
        return;
    }

    m_ui->m_switch->setChecked(checked);
}

void SettingsTrusted::updateSoftRadio(bool checked)
{
    if (!checkTrustedLoadFinied())
    {
        updateStorageMode();
        return;
    }
    m_userPin = new TrustedUserPin(this);
    m_userPin->setType(KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT);
    connect(m_userPin, &TrustedUserPin::accepted, this, &SettingsTrusted::setStorageMode);
    connect(m_userPin, &TrustedUserPin::closed, this, &SettingsTrusted::updateStorageMode);

    auto x = this->x() + this->width() / 4 + m_userPin->width() / 2;
    auto y = this->y() + this->height() / 4 + m_userPin->height() / 2;
    m_userPin->move(x, y);
    m_userPin->show();

    m_ui->m_soft->setChecked(checked);
    m_ui->m_hard->setChecked(!checked);
}

void SettingsTrusted::updateHardRadio(bool checked)
{
    if (!checkTrustedLoadFinied())
    {
        updateStorageMode();
        return;
    }
    m_userPin = new TrustedUserPin(this);
    m_userPin->setType(KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_HARD);
    connect(m_userPin, &TrustedUserPin::accepted, this, &SettingsTrusted::setStorageMode);
    connect(m_userPin, &TrustedUserPin::closed, this, &SettingsTrusted::updateStorageMode);

    auto x = this->x() + this->width() / 4 + m_userPin->width() / 2;
    auto y = this->y() + this->height() / 4 + m_userPin->height() / 2;
    m_userPin->move(x, y);
    m_userPin->show();

    m_ui->m_hard->setChecked(checked);
    m_ui->m_soft->setChecked(!checked);
}

void SettingsTrusted::setStorageMode()
{
    auto reply = m_kssDbusProxy->SetStorageMode(m_userPin->getType(), m_userPin->getUserPin());
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    updateStorageMode();
}
}  // namespace KS
