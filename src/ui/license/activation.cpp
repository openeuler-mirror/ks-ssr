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

#include "activation.h"
#include <kiran-log/qt5-log-i.h>
#include <QJsonDocument>
#include <QJsonParseError>
#include "ksc-marcos.h"
#include "src/ui/license/license-utils.h"
#include "ui_activation.h"

namespace KS
{
Activation::Activation(QWidget *parent) : TitlebarWindow(parent),
                                          m_ui(new Ui::Activation)
{
    m_ui->setupUi(getWindowContentWidget());

    m_licenseUtils = new LicenseUtils(this);
    getLicense();

    initUI();

    connect(m_ui->m_cancel, &QPushButton::clicked, this, &Activation::closed);
    connect(m_ui->m_activate, &QPushButton::clicked, this, &Activation::activate);
}

Activation::~Activation()
{
    delete m_ui;
}

bool Activation::isActivate()
{
    RETURN_VAL_IF_TRUE(m_licenseInfo.activationStatus == LAS_UNACTIVATED, false);

    auto currTime = QDateTime::currentDateTime().toSecsSinceEpoch();
    return currTime <= m_licenseInfo.expiredTime;
}

void Activation::getLicense()
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

void Activation::initUI()
{
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(440, 400);
    //创建二维码按钮
    auto hLayout = new QHBoxLayout(m_ui->m_machine_code);
    hLayout->setMargin(0);
    hLayout->setContentsMargins(12, 0, 12, 0);
    //auto qrcode

    m_ui->m_timeWidget->setVisible(isActivate());

    m_ui->m_activation_code->setText(m_licenseInfo.activationCode);
    m_ui->m_machine_code->setText(m_licenseInfo.machineCode);
    m_ui->m_expired_time->setText(QDateTime::fromSecsSinceEpoch(m_licenseInfo.expiredTime).toString("yyyy-MM-dd"));
}

void Activation::activate()
{
    auto isActived = m_licenseUtils->activateByActivationCode(m_ui->m_activation_code->text());

    //TODO:激活出错则弹出错误提示框
    emit activated(isActived);
}

}  // namespace KS
