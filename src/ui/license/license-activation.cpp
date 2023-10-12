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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "license-activation.h"
#include <kiran-log/qt5-log-i.h>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMessageBox>
#include "lib/license/license-proxy.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/license/qrcode-dialog.h"
#include "ui_license-activation.h"

#define QRCODE_PROPERTY "QRcode"
#define MACHINE_CODE QObject::tr("machine code")
#define ACTIVATION_CODE QObject::tr("activation code")

namespace KS
{
LicenseActivation::LicenseActivation(QWidget *parent) : TitlebarWindow(parent),
                                                        m_ui(new Ui::LicenseActivation),
                                                        m_licenseProxy(nullptr),
                                                        m_qrcodeDialog(nullptr)
{
    m_ui->setupUi(getWindowContentWidget());
    initUI();

    m_licenseProxy = LicenseProxy::getDefault();
    update();

    connect(m_ui->m_cancel, &QPushButton::clicked, this, &LicenseActivation::closed);
    connect(m_ui->m_activate, &QPushButton::clicked, this, &LicenseActivation::activate);

    connect(m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, &LicenseActivation::update, Qt::UniqueConnection);
}

LicenseActivation::~LicenseActivation()
{
    delete m_ui;
    if (m_qrcodeDialog)
    {
        delete m_qrcodeDialog;
        m_qrcodeDialog = nullptr;
    }
}

void LicenseActivation::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    emit closed();
}

void LicenseActivation::initUI()
{
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setTitle(tr("Activation"));
    setButtonHints(TitlebarCloseButtonHint);
    setFixedSize(469, 409);

    //创建机器码二维码按钮
    auto machineLayout = new QHBoxLayout(m_ui->m_machine_code);
    machineLayout->setMargin(0);
    machineLayout->setContentsMargins(10, 0, 10, 0);
    auto machineQRCodeBtn = new QPushButton(m_ui->m_machine_code);
    machineQRCodeBtn->setProperty(QRCODE_PROPERTY, MACHINE_CODE);
    machineQRCodeBtn->setFixedSize(16, 16);
    machineQRCodeBtn->setIcon(QIcon(":/images/qrcode"));
    machineLayout->addStretch();
    machineLayout->addWidget(machineQRCodeBtn);
    connect(machineQRCodeBtn, &QPushButton::clicked, this, &LicenseActivation::handleQrcode);

    //创建激活码二维码按钮
    auto activationLayout = new QHBoxLayout(m_ui->m_activation_code);
    activationLayout->setMargin(0);
    activationLayout->setContentsMargins(10, 0, 10, 0);
    auto activationQRcodeBtn = new QPushButton(m_ui->m_activation_code);
    activationQRcodeBtn->setCursor(Qt::ArrowCursor);
    activationQRcodeBtn->setProperty(QRCODE_PROPERTY, ACTIVATION_CODE);
    activationQRcodeBtn->setFixedSize(16, 16);
    activationQRcodeBtn->setIcon(QIcon(":/images/qrcode"));
    activationLayout->addStretch();
    activationLayout->addWidget(activationQRcodeBtn);
    connect(activationQRcodeBtn, &QPushButton::clicked, this, &LicenseActivation::handleQrcode);

    m_ui->m_machine_code->setReadOnly(true);
    m_ui->m_expired_time->setReadOnly(true);
    m_ui->m_machine_code->setTextMargins(10, 0, machineQRCodeBtn->width(), 0);
    m_ui->m_activation_code->setTextMargins(10, 0, activationQRcodeBtn->width() + activationLayout->contentsMargins().right(), 0);
    m_ui->m_expired_time->setTextMargins(10, 0, 10, 0);
}

void LicenseActivation::activate()
{
    QString errorMsg;
    auto isActivated = m_licenseProxy->activateByActivationCode(m_ui->m_activation_code->text(), errorMsg);

    POPUP_MESSAGE_DIALOG(isActivated ? tr("Activate app successful!") : errorMsg);
}

void LicenseActivation::handleQrcode()
{
    auto qrcodeBtn = qobject_cast<QPushButton *>(sender());
    auto codeStr = qrcodeBtn->property(QRCODE_PROPERTY).toString();
    auto title = tr("Scan QR code to get %1").arg(codeStr);

    if (codeStr == MACHINE_CODE)
        popupQRcode(m_licenseProxy->getMachineCode(), title);
    else
        popupQRcode(m_licenseProxy->getActivationCode(), title);
}

void LicenseActivation::popupQRcode(const QString &QRcode, const QString &title)
{
    RETURN_IF_TRUE(QRcode.isEmpty());

    if (!m_qrcodeDialog)
    {
        m_qrcodeDialog = new QRCodeDialog(this);
    }
    m_qrcodeDialog->setText(QRcode);
    m_qrcodeDialog->setSummary(title);

    auto x = this->x() + this->width() / 4 + m_qrcodeDialog->width() / 4;
    auto y = this->y() + this->height() / 4 + m_qrcodeDialog->height() / 4;
    m_qrcodeDialog->move(x, y);
    m_qrcodeDialog->raise();
    m_qrcodeDialog->show();
}

void LicenseActivation::update()
{
    m_ui->m_machine_code->setText(m_licenseProxy->getMachineCode());
    m_ui->m_activation_code->setText(m_licenseProxy->getActivationCode());
    m_ui->m_expired_time->setText(QDateTime::fromSecsSinceEpoch(m_licenseProxy->getExpiredTime()).toString("yyyy-MM-dd"));
    m_ui->m_timeWidget->setVisible(m_licenseProxy->isActivated());
}

}  // namespace KS
