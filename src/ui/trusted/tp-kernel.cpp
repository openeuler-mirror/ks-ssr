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
#include "src/ui/trusted/tp-kernel.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "src/ui/trusted/table-delete-notify.h"
#include "src/ui/trusted_proxy.h"
#include "src/ui/ui_tp-kernel.h"

namespace KS
{
TPKernel::TPKernel(QWidget *parent) : QWidget(parent),
                                      m_ui(new Ui::TPKernel)
{
    m_ui->setupUi(this);

    m_trustedProtectedProxy = new TrustedProxy(KSC_DBUS_NAME,
                                               KSC_TRUSTED_PROTECTED_DBUS_OBJECT_PATH,
                                               QDBusConnection::systemBus(),
                                               this);
    // 初始化完成自动刷新
    connect(m_trustedProtectedProxy, SIGNAL(InitFinished()), this, SLOT(updateInfo()));
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2"))
                    .arg(QString::number(m_ui->m_kernelTable->getKernelRecords().size()),
                         QString::number(m_ui->m_kernelTable->getKerneltamperedNums()));
    m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    m_ui->m_search->addAction(QIcon(":/images/search").pixmap(16, 16), QLineEdit::ActionPosition::LeadingPosition);

    connect(m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addClicked(bool)));
    connect(m_ui->m_update, SIGNAL(clicked(bool)), this, SLOT(updateClicked(bool)));
    connect(m_ui->m_unprotect, SIGNAL(clicked(bool)), this, SLOT(unprotectClicked(bool)));
}

TPKernel::~TPKernel()
{
    delete m_ui;
}

void TPKernel::updateInfo()
{
    m_ui->m_kernelTable->updateRecord();
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2"))
                    .arg(QString::number(m_ui->m_kernelTable->getKernelRecords().size()),
                         QString::number(m_ui->m_kernelTable->getKerneltamperedNums()));
    m_ui->m_tips->setText(text);
}

void TPKernel::searchTextChanged(const QString &text)
{
    m_ui->m_kernelTable->searchTextChanged(text);
}

void TPKernel::addClicked(bool checked)
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath(), "", 0, QFileDialog::DontUseCustomDirectoryIcons);
    RETURN_IF_TRUE(fileName.isEmpty())

    m_trustedProtectedProxy->AddFile(fileName).waitForFinished();
    updateInfo();
}

void TPKernel::updateClicked(bool checked)
{
    updateInfo();
}

void TPKernel::unprotectClicked(bool checked)
{
    auto unprotectNotify = new TableDeleteNotify(this);
    unprotectNotify->show();

    connect(unprotectNotify, &TableDeleteNotify::accepted, this, &TPKernel::unprotectAccepted);
}

void TPKernel::unprotectAccepted()
{
    auto trustedInfos = m_ui->m_kernelTable->getKernelRecords();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            m_trustedProtectedProxy->RemoveFile(trustedInfo.filePath).waitForFinished();
        }
    }
    updateInfo();
}

}  // namespace KS
