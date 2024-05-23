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

#include "tp-setting-page.h"
#include <ssr-i.h>
#include "kss_dbus_proxy.h"
#include "lib/widgets/ssr-marcos-ui.h"
#include "tp-user-pin.h"
#include "ui_tp-setting-page.h"

namespace KS
{
namespace TP
{
TPSettingPage::TPSettingPage(QWidget *parent)
    : SettingPage(parent),
      m_ui(new Ui::TPSettingPage)
{
    m_ui->setupUi(this);

    m_kssDbusProxy = new KSSDbusProxy(SSR_DBUS_NAME,
                                      SSR_KSS_INIT_DBUS_OBJECT_PATH,
                                      QDBusConnection::systemBus(),
                                      this);

    initUI();
}

TPSettingPage::~TPSettingPage()
{
    delete m_ui;
}

QString TPSettingPage::getTitle()
{
    return tr("Trusted protect");
}

void TPSettingPage::initUI()
{
    // switch
    m_ui->m_switch->setCheckable(true);
    m_ui->m_switch->setChecked(m_kssDbusProxy->trustedStatus());
    m_ui->m_switch->setFixedSize(52, 24);
    m_ui->m_switch->setIconSize(QSize(52, 24));
    connect(m_ui->m_switch, &QPushButton::clicked, this, &TPSettingPage::setTrustedStatus);

    // radio
    m_ui->m_soft->setCheckable(true);
    m_ui->m_soft->setIconSize(QSize(14, 14));

    m_ui->m_hard->setCheckable(true);
    m_ui->m_hard->setIconSize(QSize(14, 14));
    m_ui->m_hardLabel->setWordWrap(true);

    updateStorageMode();

    connect(m_ui->m_soft, &QPushButton::clicked, this, &TPSettingPage::updateSoftRadio);
    connect(m_ui->m_hard, &QPushButton::clicked, this, &TPSettingPage::updateHardRadio);
}

void TPSettingPage::updateStorageMode()
{
    // 通过后台是否设置成功修改按钮状态
    auto mode = m_kssDbusProxy->storageMode();
    m_ui->m_soft->setChecked(mode == SSRKSSTrustedStorageType::SSR_KSS_TRUSTED_STORAGE_TYPE_SOFT ? true : false);
    m_ui->m_hard->setChecked(mode == SSRKSSTrustedStorageType::SSR_KSS_TRUSTED_STORAGE_TYPE_SOFT ? false : true);
}

bool TPSettingPage::checkTrustedLoadFinied()
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

void TPSettingPage::setTrustedStatus(bool checked)
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

void TPSettingPage::updateSoftRadio(bool checked)
{
    // 选中状态单击后QPushButton会将按钮状态置为false，当在选中状态点击按钮时不做处理，并将按钮状态改回去
    if (!m_ui->m_soft->isChecked())
    {
        m_ui->m_soft->setChecked(true);
        return;
    }
    if (!checkTrustedLoadFinied())
    {
        updateStorageMode();
        return;
    }
    m_userPin = new TPUserPin(this);
    m_userPin->setType(SSRKSSTrustedStorageType::SSR_KSS_TRUSTED_STORAGE_TYPE_SOFT);
    connect(m_userPin, &TPUserPin::accepted, this, &TPSettingPage::setStorageMode);
    connect(m_userPin, &TPUserPin::closed, this, &TPSettingPage::updateStorageMode);

    auto x = window()->x() + window()->width() / 2 - m_userPin->width() / 2;
    auto y = window()->y() + window()->height() / 2 - m_userPin->height() / 2;
    m_userPin->move(x, y);
    m_userPin->show();
    // 没有设置成功前，不应将按钮选中
    m_ui->m_soft->setChecked(!checked);
}

void TPSettingPage::updateHardRadio(bool checked)
{
    // 选中状态单击后QPushButton会将按钮状态置为false，当在选中状态点击按钮时不做处理，并将按钮状态改回去
    if (!m_ui->m_hard->isChecked())
    {
        m_ui->m_hard->setChecked(true);
        return;
    }
    if (!checkTrustedLoadFinied())
    {
        updateStorageMode();
        return;
    }
    m_userPin = new TPUserPin(this);
    m_userPin->setType(SSRKSSTrustedStorageType::SSR_KSS_TRUSTED_STORAGE_TYPE_HARD);
    connect(m_userPin, &TPUserPin::accepted, this, &TPSettingPage::setStorageMode);
    connect(m_userPin, &TPUserPin::closed, this, &TPSettingPage::updateStorageMode);

    auto x = this->x() + this->width() / 4 + m_userPin->width() / 2;
    auto y = this->y() + this->height() / 4 + m_userPin->height() / 2;
    m_userPin->move(x, y);
    m_userPin->show();
    // 没有设置成功前，不应将按钮选中
    m_ui->m_hard->setChecked(!checked);
}

void TPSettingPage::setStorageMode()
{
    auto reply = m_kssDbusProxy->SetStorageMode(m_userPin->getType(), m_userPin->getUserPin());
    CHECK_ERROR_FOR_DBUS_REPLY_AND_RETURN(reply);
    updateStorageMode();
}
}  // namespace TP
}  // namespace KS
