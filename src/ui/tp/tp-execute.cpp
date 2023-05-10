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
#include "src/ui/tp/tp-execute.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "src/ui/common/message-dialog.h"
#include "src/ui/tp/table-delete-notify.h"
#include "src/ui/tp_proxy.h"
#include "src/ui/ui_tp-execute.h"

namespace KS
{
TPExecute::TPExecute(QWidget *parent) : QWidget(parent),
                                        m_ui(new Ui::TPExecute)
{
    m_ui->setupUi(this);
    m_ui->m_note->setWordWrap(true);
    m_dbusProxy = new TPProxy(KSC_DBUS_NAME,
                              KSC_TP_DBUS_OBJECT_PATH,
                              QDBusConnection::systemBus(),
                              this);
    // 初始化完成自动刷新
    connect(m_dbusProxy, &TPProxy::InitFinished, this, [this]
            {
                updateInfo();
                emit initFinished();
            });
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2")).arg(QString::number(m_ui->m_executeTable->getExecuteRecords().size()), QString::number(m_ui->m_executeTable->getExecutetamperedNums()));
    m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    m_ui->m_search->addAction(QIcon(":/images/search").pixmap(16, 16), QLineEdit::ActionPosition::LeadingPosition);

    connect(m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addClicked(bool)));
    connect(m_ui->m_update, SIGNAL(clicked(bool)), this, SLOT(updateClicked(bool)));
    connect(m_ui->m_unprotect, SIGNAL(clicked(bool)), this, SLOT(unprotectClicked(bool)));
}

TPExecute::~TPExecute()
{
    delete m_ui;
}

void TPExecute::updateInfo()
{
    m_ui->m_executeTable->updateRecord();
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2")).arg(QString::number(m_ui->m_executeTable->getExecuteRecords().size()), QString::number(m_ui->m_executeTable->getExecutetamperedNums()));
    m_ui->m_tips->setText(text);
}

void TPExecute::searchTextChanged(const QString &text)
{
    m_ui->m_executeTable->searchTextChanged(text);
}

void TPExecute::addClicked(bool checked)
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath());
    RETURN_IF_TRUE(fileName.isEmpty())

    auto reply = m_dbusProxy->AddFile(fileName);
    reply.waitForFinished();

    if (reply.isError())
    {
        KLOG_WARNING() << "Failed to add files: " << reply.error().message();
        auto message = new MessageDialog(this);
        message->setFixedSize(240, 200);
        message->buildNotify(reply.error().message());

        int x = this->x() + this->width() / 4 + message->width() / 4;
        int y = this->y() + this->height() / 4 + message->height() / 4;
        message->move(x, y);
        message->show();
    }
    updateInfo();
}

void TPExecute::updateClicked(bool checked)
{
    updateInfo();
}

void TPExecute::unprotectClicked(bool checked)
{
    auto unprotectNotify = new TableDeleteNotify(this);
    unprotectNotify->show();

    connect(unprotectNotify, &TableDeleteNotify::accepted, this, &TPExecute::unprotectAccepted);
}

void TPExecute::unprotectAccepted()
{
    auto trustedInfos = m_ui->m_executeTable->getExecuteRecords();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            m_dbusProxy->RemoveFile(trustedInfo.filePath).waitForFinished();
        }
    }
    updateInfo();
}

}  // namespace KS
