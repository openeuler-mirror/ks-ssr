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
#include "src/ui/trusted/tp-execute.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include "ksc-i.h"
#include "src/ui/trusted_proxy.h"
#include "src/ui/ui_tp-execute.h"

namespace KS
{
TPExecute::TPExecute(QWidget *parent) : QWidget(parent),
                                        m_ui(new Ui::TPExecute)
{
    this->m_ui->setupUi(this);

    this->m_trustedProtectedProxy = new TrustedProxy(KSC_DBUS_NAME,
                                                     KSC_FILE_PROTECTED_DBUS_OBJECT_PATH,
                                                     QDBusConnection::systemBus(),
                                                     this);
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2")).arg(QString::number(this->m_ui->m_executeTable->getExecuteRecords().size()),
                                                                                 QString::number(this->m_ui->m_executeTable->getExecutetamperedNums()));
    this->m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    this->m_ui->m_search->addAction(QIcon(":/images/search"), QLineEdit::ActionPosition::LeadingPosition);

    connect(this->m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(this->m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addClicked(bool)));
    connect(this->m_ui->m_update, SIGNAL(clicked(bool)), this, SLOT(updateClicked(bool)));
    connect(this->m_ui->m_unprotect, SIGNAL(clicked(bool)), this, SLOT(unprotectClicked(bool)));
}

TPExecute::~TPExecute()
{
    delete m_ui;
}

void TPExecute::updateInfo()
{
    this->m_ui->m_executeTable->updateRecord();
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2")).arg(QString::number(this->m_ui->m_executeTable->getExecuteRecords().size()),
                                                                                 QString::number(this->m_ui->m_executeTable->getExecutetamperedNums()));
    this->m_ui->m_tips->setText(text);
}

void TPExecute::searchTextChanged(const QString &text)
{
    this->m_ui->m_executeTable->searchTextChanged(text);
}

void TPExecute::addClicked(bool checked)
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath());
    if (!fileName.isEmpty())
    {
        this->m_trustedProtectedProxy->AddFile(fileName);
        updateInfo();
    }
}

void TPExecute::updateClicked(bool checked)
{
    updateInfo();
}

void TPExecute::unprotectClicked(bool checked)
{
    auto trustedInfos = this->m_ui->m_executeTable->getExecuteRecords();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            this->m_trustedProtectedProxy->RemoveFile(trustedInfo.filePath);
        }
    }
    updateInfo();
}

}  // namespace KS
