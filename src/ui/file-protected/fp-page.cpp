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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/ui/file-protected/fp-page.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include "ksc-i.h"
#include "src/ui/file_protected_proxy.h"
#include "src/ui/ui_fp-page.h"

namespace KS
{
FPPage::FPPage(QWidget *parent) : QWidget(parent),
                   m_ui(new Ui::FPPage())
{
    this->m_ui->setupUi(this);

    this->m_fileProtectedProxy = new FileProtectedProxy(KSC_DBUS_NAME,
                                                        KSC_FILE_PROTECTED_DBUS_OBJECT_PATH,
                                                        QDBusConnection::systemBus(),
                                                        this);
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(this->m_ui->m_fileTable->getFPFileInfos().size());
    this->m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    this->m_ui->m_search->addAction(QIcon(":/images/search"), QLineEdit::ActionPosition::LeadingPosition);

    connect(this->m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(this->m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addClicked(bool)));
    connect(this->m_ui->m_update, SIGNAL(clicked(bool)), this, SLOT(updateClicked(bool)));
    connect(this->m_ui->m_unprotect, SIGNAL(clicked(bool)), this, SLOT(unprotectClicked(bool)));
}

FPPage::~FPPage()
{
    delete m_ui;
}

void FPPage::searchTextChanged(const QString &text)
{
    this->m_ui->m_fileTable->searchTextChanged(text);
}

void FPPage::addClicked(bool checked)
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath());
    if (!fileName.isEmpty())
    {
        this->m_fileProtectedProxy->AddFile(fileName);
        //        this->m_ui->m_fileTable->updateInfo();
    }
}

void FPPage::updateClicked(bool checked)
{
    this->m_ui->m_fileTable->updateInfo();
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(this->m_ui->m_fileTable->getFPFileInfos().size());
    this->m_ui->m_tips->setText(text);
}

void FPPage::unprotectClicked(bool checked)
{
    auto trustedInfos = this->m_ui->m_fileTable->getFPFileInfos();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            this->m_fileProtectedProxy->RemoveFile(trustedInfo.filePath);
        }
    }
    this->m_ui->m_fileTable->updateInfo();
}

}  // namespace KS
