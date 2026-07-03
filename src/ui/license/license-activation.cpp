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
#include "src/ui/license/license-dbus.h"
#include "src/ui/license/qrcode-dialog.h"
#include "ui_license-activation.h"

namespace KS
{
LicenseActivation::LicenseActivation(QWidget *parent) : TitlebarWindow(parent),
                                                        m_ui(new Ui::LicenseActivation),
                                                        m_licenseUtils(nullptr),
                                                        m_qrcodeDialog(nullptr)
{
    m_ui->setupUi(getWindowContentWidget());
    initUI();

    m_licenseUtils = new LicenseDBus(this);
    updateLicenseInfo();

    connect(m_ui->m_cancel, &QPushButton::clicked, this, &LicenseActivation::closed);
    connect(m_ui->m_activate, &QPushButton::clicked, this, &LicenseActivation::activate);
    connect(m_licenseUtils, &LicenseDBus::licenseChanged, this, &LicenseActivation::getLicense);
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

bool LicenseActivation::isActivate()
{
    RETURN_VAL_IF_TRUE(m_licenseInfo.activationStatus != LAS_ACTIVATED, false);

    //判断激活是否过期
    auto currTime = QDateTime::currentDateTime().toSecsSinceEpoch();
    return currTime <= m_licenseInfo.expiredTime;
}

void LicenseActivation::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    emit closed();
}

void LicenseActivation::getLicenseInfo()
{
    auto licenseInfoJson = m_licenseUtils->getLicense();
    RETURN_IF_TRUE(licenseInfoJson.isEmpty());

    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(licenseInfoJson.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files information failed: " << jsonError.errorString();
        return;
    }
    else
    {
        auto data = jsonDoc.object();
        m_licenseInfo.activationCode = data.value(LICENSE_JK_ACTIVATION_CODE).toString();
        m_licenseInfo.machineCode = data.value(LICENSE_JK_MACHINE_CODE).toString();
        m_licenseInfo.activationStatus = data.value(LICENSE_JK_ACTIVATION_STATUS).toInt();
        m_licenseInfo.expiredTime = time_t(data.value(LICENSE_JK_EXPIRED_TIME).toVariant().toUInt());
    }
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
    auto isActivated = m_licenseUtils->activateByActivationCode(m_ui->m_activation_code->text(), errorMsg);

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
        QMessageBox::warning(this,
                             tr("Activate app"),
                             tr("Activate app successful!"),
                             QMessageBox::Ok);
    }
}

void LicenseActivation::popupQrencode()
{
    RETURN_IF_TRUE(m_licenseInfo.machineCode.isEmpty());

    if (!m_qrcodeDialog)
    {
        m_qrcodeDialog = new QRCodeDialog(this);
        m_qrcodeDialog->setText(m_licenseInfo.machineCode);
    }

    auto x = this->x() + this->width() / 4 + m_qrcodeDialog->width() / 4;
    auto y = this->y() + this->height() / 4 + m_qrcodeDialog->height() / 4;
    m_qrcodeDialog->move(x, y);
    m_qrcodeDialog->raise();
    m_qrcodeDialog->show();
}

void LicenseActivation::getLicense(bool isChanged)
{
    if (isChanged)
    {
        updateLicenseInfo();
        emit activated(isActivate());
    }
}

void LicenseActivation::updateLicenseInfo()
{
    getLicenseInfo();
    setLicenseInfo();
}

void LicenseActivation::setLicenseInfo()
{
    m_ui->m_machine_code->setText(m_licenseInfo.machineCode);
    m_ui->m_activation_code->setText(m_licenseInfo.activationCode);
    m_ui->m_expired_time->setText(QDateTime::fromSecsSinceEpoch(m_licenseInfo.expiredTime).toString("yyyy-MM-dd"));
    m_ui->m_timeWidget->setVisible(isActivate());
}

}  // namespace KS
