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

#include "trusted-view.h"
#include <QFileDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QSettings>
#include "config.h"
#include "sc-i.h"
#include "sc-marcos.h"
#include "src/ui/common/custom-window.h"
#include "src/ui/trusted_proxy.h"

namespace KS
{
TrustedView::TrustedView(QWidget *parent, TrustedProtectType type) : TableCommon(parent),
                                                                     m_type(type)
{
    this->m_trustedProxy = new TrustedProxy(SC_DBUS_NAME,
                                            SC_TRUSTED_PROTECTED_DBUS_OBJECT_PATH,
                                            QDBusConnection::systemBus(),
                                            this);
    this->initBtns();
    this->initUI();
    this->checkInitDataIsFinished();

    connect(this, &TableCommon::sigSearchTextChanged, this, &TrustedView::searchTextChanged);
    connect(m_trustedProxy, &TrustedProxy::InitFinished, this, &TrustedView::initFinished);
}

void TrustedView::initUI()
{
    setPrompt((m_type == TrustedProtectType::TRUSTED_PROTECT_KERNEL) ? tr("Manage kernel driver modules to prevent illegal loading and uninstallation") : tr("System core component integrity protection,protection and implementation environment safety"));
    m_trustedTable = new TPTable(this, m_type);
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_trustedTable->getTrustedInfos().size());
    setSumText(text);
    addTable(m_trustedTable);
}

void TrustedView::initBtns()
{
    if (!m_operateBtnList.isEmpty())
    {
        m_operateBtnList.clear();
    }
    auto *addFileBtn = new QPushButton(this);
    addFileBtn->setText(tr("Add file"));
    addFileBtn->setFixedSize(72, 36);
    addFileBtn->setObjectName("m_add");

    auto *updateBtn = new QPushButton(this);
    updateBtn->setText(tr("Update file"));
    updateBtn->setFixedSize(72, 36);
    updateBtn->setObjectName("m_update");

    auto *delFileBtn = new QPushButton(this);
    delFileBtn->setText(tr("Del file"));
    delFileBtn->setFixedSize(72, 36);
    delFileBtn->setObjectName("m_unprotect");

    connect(addFileBtn, SIGNAL(clicked(bool)), this, SLOT(addClicked(bool)));
    connect(updateBtn, SIGNAL(clicked(bool)), this, SLOT(updateClicked(bool)));
    connect(delFileBtn, SIGNAL(clicked(bool)), this, SLOT(delClicked(bool)));

    m_operateBtnList << addFileBtn << updateBtn << delFileBtn;

    addOperationButton(m_operateBtnList);
}

void TrustedView::checkInitDataIsFinished()
{
    auto settings = new QSettings(KSS_INI_PATH, QSettings::IniFormat, this);
    RETURN_IF_TRUE(settings->value(KSS_INI_KEY).toInt() != 0)
    // 仅在首次启动、首页显示
    RETURN_IF_TRUE(m_type != TrustedProtectType::TRUSTED_PROTECT_EXECUTE)
    auto notify = new CustomWindow(this);
    notify->setFixedSize(240, 180);
    notify->buildNotify(tr("Trusted data needs to be initialised,"
                           "please wait a few minutes to refresh."));

    int x = this->x() + this->width() / 4 + notify->width() / 4;
    int y = this->y() + this->height() / 4 + notify->height() / 4;
    notify->move(x, y);
    notify->show();
}

void TrustedView::addClicked(bool checked)
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath());
    if (!fileName.isEmpty())
    {
        this->m_trustedProxy->AddFile(fileName);
        m_trustedTable->updateInfo();
    }
}

void TrustedView::updateClicked(bool checked)
{
    m_trustedTable->updateInfo();
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_trustedTable->getTrustedInfos().size());
    setSumText(text);
}

void TrustedView::delClicked(bool checked)
{
    auto trustedInfos = m_trustedTable->getTrustedInfos();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            this->m_trustedProxy->RemoveFile(trustedInfo.filePath);
        }
    }
    m_trustedTable->updateInfo();
}

void TrustedView::searchTextChanged(const QString &text)
{
    m_trustedTable->searchTextChanged(text);
}

void TrustedView::initFinished()
{
    m_trustedTable->updateInfo();
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_trustedTable->getTrustedInfos().size());
    setSumText(text);
}

}  // namespace KS
