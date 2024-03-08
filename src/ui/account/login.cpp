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

#include "login.h"
#include <QAction>
#include <QCloseEvent>
#include <QComboBox>
#include <QIcon>
#include <QList>
#include <QStyledItemDelegate>
#include "common/password-event-filter.h"
#include "include/ssr-i.h"
#include "lib/license/license-proxy.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/license/activation.h"
#include "ui_login.h"

namespace KS
{
namespace Account
{
Login::Login(QWidget *parent)
    : TitlebarWindow(parent),
      m_ui(new Ui::Login)
{
    m_ui->setupUi(getWindowContentWidget());
    initUI();
}

Login::~Login()
{
    delete m_ui;
}

QString Login::getPassword() const
{
    return m_ui->m_password->text();
}

void Login::setPassword(const QString &password)
{
    m_ui->m_password->setText(password);
}

QString Login::getAccountName() const
{
    return m_ui->m_comboBox->currentText();
}

void Login::setAccountName(const QString &name)
{
    m_ui->m_comboBox->setCurrentText(name);
}

void Login::closeEvent(QCloseEvent *event)
{
    emit rejected();
    TitlebarWindow::closeEvent(event);
}

void Login::initUI()
{
    m_licenseProxy = LicenseProxy::getDefault();
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setTitle(tr("Security reinforcement"));
    setMinimumSize(400, 320);
    setTitlebarCustomLayoutAlignHCenter(false);
    auto layout = getTitlebarCustomLayout();
    layout->setContentsMargins(0, 0, 4, 0);
    // 未激活文本
    m_activateStatus = new QPushButton(this);
    m_activateStatus->setObjectName("activateStatus");
    m_activateStatus->setFixedHeight(18);
    m_activateStatus->setText(tr("Unactivated"));
    m_licenseProxy->isActivated() ? m_activateStatus->hide() : m_activateStatus->show();
    connect(m_activateStatus, &QPushButton::clicked, this, &Login::popupActiveDialog);
    layout->addWidget(m_activateStatus);
    layout->setAlignment(Qt::AlignRight);

    connect(
        m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, [this]
        {
            m_activateStatus->setVisible(!m_licenseProxy->isActivated());
        },
        Qt::UniqueConnection);

    // 给QCombobox设置代理才能设置下拉列表项的高度
    auto delegate = new QStyledItemDelegate(this);
    m_ui->m_comboBox->setItemDelegate(delegate);
    m_ui->m_comboBox->addItems(QStringList() << SSR_ACCOUNT_NAME_SYSADM
                                             << SSR_ACCOUNT_NAME_SECADM
                                             << SSR_ACCOUNT_NAME_AUDADM);
    connect(m_ui->m_cancel, &QPushButton::clicked, this, &Login::close);
    connect(m_ui->m_ok, &QPushButton::clicked, this, [this]
            {
                m_licenseProxy->isActivated() ? emit accepted() : popupActiveDialog();
            });
    connect(m_ui->m_password, &QLineEdit::returnPressed, this, [this]
            {
                m_licenseProxy->isActivated() ? emit accepted() : popupActiveDialog();
            });
    // 限制字符
    m_ui->m_password->setMaxLength(SSR_PASSWORD_MAX_LENGTH);
    m_ui->m_password->setEchoMode(QLineEdit::Password);
    m_ui->m_password->setContextMenuPolicy(Qt::NoContextMenu);
    m_ui->m_password->installEventFilter(new PasswordEventFilter(m_ui->m_password));
}

void Login::popupActiveDialog()
{
    auto activation = new Activation::Activation(this);
    connect(activation, &Activation::Activation::activated, [this](const QString &message)
            {
                POPUP_MESSAGE_DIALOG(message);
            });

    auto x = this->x() + this->width() / 2 - activation->width() / 2;
    auto y = this->y() + this->height() / 2 - activation->height() / 2;
    activation->move(x, y);
    activation->show();
}

}  // namespace Account
}  // namespace KS
