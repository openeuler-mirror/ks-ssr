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
#include "include/ksc-marcos.h"
#include "src/ui/common/message-dialog.h"
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

    init();
}

SettingsTrusted::~SettingsTrusted()
{
    delete m_ui;
}

void SettingsTrusted::init()
{
    // switch
    m_ui->m_switch->setCheckable(true);
    m_ui->m_switch->setChecked(m_kssDbusProxy->trustedStatus());
    m_ui->m_switch->setIcon(QIcon(m_ui->m_switch->isChecked() ? ":/images/switch-open" : ":/images/switch-close"));
    m_ui->m_switch->setFixedSize(52, 24);
    m_ui->m_switch->setIconSize(QSize(52, 24));
    connect(m_ui->m_switch, &QPushButton::clicked, this, &SettingsTrusted::switchChanged);

    // radio
    auto mode = m_kssDbusProxy->storageMode();
    m_ui->m_soft->setCheckable(true);
    m_ui->m_soft->setIcon(QIcon(mode == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? ":/images/radio-checked" : ":/images/radio-unchecked"));
    m_ui->m_soft->setChecked(mode == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? true : false);
    m_ui->m_soft->setIconSize(QSize(14, 14));

    m_ui->m_hard->setCheckable(true);
    m_ui->m_hard->setIcon(QIcon(mode == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? ":/images/radio-unchecked" : ":/images/radio-checked"));
    m_ui->m_soft->setChecked(mode == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? false : true);
    m_ui->m_hard->setIconSize(QSize(14, 14));
    m_ui->m_hardLabel->setWordWrap(true);

    connect(m_ui->m_soft, &QPushButton::clicked, this, &SettingsTrusted::softRadioChanged);
    connect(m_ui->m_hard, &QPushButton::clicked, this, &SettingsTrusted::hardRadioChanged);
}

void SettingsTrusted::updateSoftMode()
{
    auto mode = m_kssDbusProxy->storageMode();
    m_ui->m_soft->setIcon(QIcon(mode == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? ":/images/radio-checked" : ":/images/radio-unchecked"));
    m_ui->m_soft->setChecked(mode == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? true : false);

    m_ui->m_hard->setIcon(QIcon(mode == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? ":/images/radio-unchecked" : ":/images/radio-checked"));
    m_ui->m_soft->setChecked(mode == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? false : true);
}

bool SettingsTrusted::checkTrustedLoadFinied()
{
    // 可信未初始化完成，不允许操作
    if (!m_kssDbusProxy->initialized())
    {
        auto messgeDialog = new MessageDialog(this);
        messgeDialog->setMessage(tr("Trusted data needs to be initialised,"
                                    "please wait a few minutes before trying."));

        int x = this->x() + width() / 4 + messgeDialog->width() / 4;
        int y = this->y() + height() / 4 + messgeDialog->height() / 4;
        messgeDialog->move(x, y);
        messgeDialog->show();
        return false;
    }
    return true;
}

void SettingsTrusted::switchChanged(bool checked)
{
    RETURN_IF_TRUE(!checkTrustedLoadFinied())
    auto reply = m_kssDbusProxy->SetTrustedStatus(checked);
    reply.waitForFinished();

    if (reply.isError())
    {
        auto messageDialog = new MessageDialog(this);
        messageDialog->setMessage(reply.error().message());

        auto x = this->x() + this->width() / 4 + messageDialog->width() / 2;
        auto y = this->y() + this->height() / 4 + messageDialog->height() / 2;
        messageDialog->move(x, y);
        messageDialog->show();
        return;
    }

    m_ui->m_switch->setIcon(QIcon(checked ? ":/images/switch-open" : ":/images/switch-close"));
    m_ui->m_switch->setChecked(checked);
}

void SettingsTrusted::softRadioChanged(bool checked)
{
    RETURN_IF_TRUE(!checkTrustedLoadFinied())
    m_userPin = new TrustedUserPin(this);
    connect(m_userPin, &TrustedUserPin::accepted, this, &SettingsTrusted::inputSoftUserPinAccepted);
    connect(m_userPin, &TrustedUserPin::rejected, this, [this]
            {
                // 取消需将选中状态改回去
                updateSoftMode();
            });

    auto x = this->x() + this->width() / 4 + m_userPin->width() / 2;
    auto y = this->y() + this->height() / 4 + m_userPin->height() / 2;
    m_userPin->move(x, y);
    m_userPin->show();

    m_ui->m_soft->setIcon(QIcon(checked ? ":/images/radio-checked" : ":/images/radio-unchecked"));
    m_ui->m_soft->setChecked(checked);

    m_ui->m_hard->setIcon(QIcon(!checked ? ":/images/radio-checked" : ":/images/radio-unchecked"));
    m_ui->m_hard->setChecked(!checked);
}

void SettingsTrusted::hardRadioChanged(bool checked)
{
    RETURN_IF_TRUE(!checkTrustedLoadFinied())
    m_userPin = new TrustedUserPin(this);
    connect(m_userPin, &TrustedUserPin::accepted, this, &SettingsTrusted::inputHardUserPinAccepted);
    connect(m_userPin, &TrustedUserPin::rejected, this, [this]
            {
                // 取消需将选中状态改回去
                updateSoftMode();
            });

    auto x = this->x() + this->width() / 4 + m_userPin->width() / 2;
    auto y = this->y() + this->height() / 4 + m_userPin->height() / 2;
    m_userPin->move(x, y);
    m_userPin->show();

    m_ui->m_hard->setIcon(QIcon(checked ? ":/images/radio-checked" : ":/images/radio-unchecked"));
    m_ui->m_hard->setChecked(checked);

    m_ui->m_soft->setIcon(QIcon(!checked ? ":/images/radio-checked" : ":/images/radio-unchecked"));
    m_ui->m_soft->setChecked(!checked);
}

void SettingsTrusted::inputSoftUserPinAccepted()
{
    auto reply = m_kssDbusProxy->SetStorageMode(KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT, m_userPin->getUserPin());
    reply.waitForFinished();

    if (reply.isError())
    {
        auto messageDialog = new MessageDialog(this);
        messageDialog->setMessage(reply.error().message());

        auto x = this->x() + this->width() / 4 + messageDialog->width() / 2;
        auto y = this->y() + this->height() / 4 + messageDialog->height() / 2;
        messageDialog->move(x, y);
        messageDialog->show();
        // 错误则将选中状态改回去
        updateSoftMode();
        return;
    }
}

void SettingsTrusted::inputHardUserPinAccepted()
{
    auto reply = m_kssDbusProxy->SetStorageMode(KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_HARD, m_userPin->getUserPin());
    reply.waitForFinished();

    if (reply.isError())
    {
        auto messageDialog = new MessageDialog(this);
        messageDialog->setMessage(reply.error().message());

        auto x = this->x() + this->width() / 4 + messageDialog->width() / 2;
        auto y = this->y() + this->height() / 4 + messageDialog->height() / 2;
        messageDialog->move(x, y);
        messageDialog->show();
        // 错误则将选中状态改回去
        updateSoftMode();
        return;
    }
}
}  // namespace KS
