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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "license-activation.h"
#include <kiran-log/qt5-log-i.h>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMessageBox>
#include "include/ksc-marcos.h"
#include "src/ui/license/qrcode-dialog.h"
#include "ui_license-activation.h"

namespace KS
{
LicenseActivation::LicenseActivation(QWidget *parent) : TitlebarWindow(parent),
                                                        m_ui(new Ui::LicenseActivation),
                                                        m_licenseDBus(nullptr),
                                                        m_qrcodeDialog(nullptr),
                                                        m_licenseInfo(nullptr)
{
    m_ui->setupUi(getWindowContentWidget());
    initUI();

    m_licenseDBus = LicenseDBus::getDefault();
    update();

    connect(m_ui->m_cancel, &QPushButton::clicked, this, &LicenseActivation::closed);
    connect(m_ui->m_activate, &QPushButton::clicked, this, &LicenseActivation::activate);

    connect(m_licenseDBus.data(), &LicenseDBus::licenseChanged, this, &LicenseActivation::setLicense, Qt::UniqueConnection);
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
    setFixedSize(450, 390);

    //创建二维码按钮
    auto hLayout = new QHBoxLayout(m_ui->m_machine_code);
    hLayout->setMargin(0);
    hLayout->setContentsMargins(10, 0, 10, 0);
    auto qrcodeBtn = new QPushButton(m_ui->m_machine_code);
    qrcodeBtn->setFixedSize(16, 16);
    qrcodeBtn->setIcon(QIcon(":/images/qrcode"));
    hLayout->addStretch();
    hLayout->addWidget(qrcodeBtn);
    connect(qrcodeBtn, &QPushButton::clicked, this, &LicenseActivation::popupQrencode);

    m_ui->m_machine_code->setReadOnly(true);
    m_ui->m_expired_time->setReadOnly(true);
    m_ui->m_machine_code->setTextMargins(10, 0, qrcodeBtn->width(), 0);
    m_ui->m_activation_code->setTextMargins(10, 0, 10, 0);
    m_ui->m_expired_time->setTextMargins(10, 0, 10, 0);
}

void LicenseActivation::activate()
{
    QString errorMsg;
    auto isActivated = m_licenseDBus->activateByActivationCode(m_ui->m_activation_code->text(), errorMsg);

    //TODO:弹出自定义错误提示框
    if (!isActivated)
    {
        QMessageBox::warning(this,
                             tr("Activate app"),
                             errorMsg,
                             QMessageBox::Ok);
    }
    else
    {
        QMessageBox::information(this,
                                 tr("Activate app"),
                                 tr("Activate app successful!"),
                                 QMessageBox::Ok);
    }
}

void LicenseActivation::popupQrencode()
{
    RETURN_IF_TRUE(m_licenseInfo.data()->machineCode.isEmpty());

    if (!m_qrcodeDialog)
    {
        m_qrcodeDialog = new QRCodeDialog(this);
    }

    m_qrcodeDialog->setText(m_licenseInfo.data()->machineCode);
    auto x = this->x() + this->width() / 4 + m_qrcodeDialog->width() / 4;
    auto y = this->y() + this->height() / 4 + m_qrcodeDialog->height() / 4;
    m_qrcodeDialog->move(x, y);
    m_qrcodeDialog->raise();
    m_qrcodeDialog->show();
}

void LicenseActivation::setLicense(QSharedPointer<LicenseInfo> licenseInfo)
{
    m_licenseInfo = licenseInfo;
    m_ui->m_machine_code->setText(m_licenseInfo.data()->machineCode);
    m_ui->m_activation_code->setText(m_licenseInfo.data()->activationCode);
    m_ui->m_expired_time->setText(QDateTime::fromSecsSinceEpoch(m_licenseInfo.data()->expiredTime).toString("yyyy-MM-dd"));
    m_ui->m_timeWidget->setVisible(m_licenseDBus->isActivated());
}

void LicenseActivation::update()
{
    auto licenseInfo = m_licenseDBus->getLicense();
    setLicense(licenseInfo);
}

}  // namespace KS
