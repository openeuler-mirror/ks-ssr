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

#include "src/ui/fp/fp-page.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include "config.h"
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "src/ui/common/message-dialog.h"
#include "src/ui/file_protected_proxy.h"
#include "src/ui/tp/table-delete-notify.h"
#include "src/ui/ui_fp-page.h"

namespace KS
{
// ini文件
#define KSC_INI_PATH KSC_INSTALL_DATADIR "/ksc.ini"
#define KSC_INI_KEY "ksc/initialized"

FPPage::FPPage(QWidget *parent) : QWidget(parent),
                                  m_ui(new Ui::FPPage())
{
    m_ui->setupUi(this);

    m_fileProtectedProxy = new FileProtectedProxy(KSC_DBUS_NAME,
                                                  KSC_FP_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_fileTable->getFPFileInfos().size());
    m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    m_ui->m_search->addAction(QIcon(":/images/search").pixmap(16, 16), QLineEdit::ActionPosition::LeadingPosition);

    connect(m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addClicked(bool)));
    //    connect(m_ui->m_update, SIGNAL(clicked(bool)), this, SLOT(updateClicked(bool)));
    connect(m_ui->m_unprotect, SIGNAL(clicked(bool)), this, SLOT(unprotectClicked(bool)));
}

FPPage::~FPPage()
{
    delete m_ui;
}

void FPPage::searchTextChanged(const QString &text)
{
    m_ui->m_fileTable->searchTextChanged(text);
}

void FPPage::addClicked(bool checked)
{
    RETURN_IF_TRUE(!checkTrustedLoadFinied())
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath());
    if (!fileName.isEmpty())
    {
        m_fileProtectedProxy->AddFile(fileName).waitForFinished();
        //        m_ui->m_fileTable->updateInfo();
        updateInfo();
    }
}

void FPPage::updateInfo()
{
    m_ui->m_fileTable->updateInfo();
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_fileTable->getFPFileInfos().size());
    m_ui->m_tips->setText(text);
}

bool FPPage::checkTrustedLoadFinied()
{
    // 可信未初始化完成，不允许操作
    auto settings = new QSettings(KSC_INI_PATH, QSettings::IniFormat, this);
    if (settings->value(KSC_INI_KEY).toInt() == 0)
    {
        auto message = new MessageDialog(this);
        message->setFixedSize(240, 200);
        message->buildNotify(tr("Trusted data needs to be initialised,"
                                "please wait a few minutes before trying."));

        int x = this->x() + width() / 4 + message->width() / 4;
        int y = this->y() + height() / 4 + message->height() / 4;
        message->move(x, y);
        message->show();
        return false;
    }
    return true;
}

void FPPage::unprotectClicked(bool checked)
{
    RETURN_IF_TRUE(!checkTrustedLoadFinied())
    auto unprotectNotify = new TableDeleteNotify(this);
    unprotectNotify->show();

    connect(unprotectNotify, &TableDeleteNotify::accepted, this, &FPPage::unprotectAccepted);
}

void FPPage::unprotectAccepted()
{
    auto trustedInfos = m_ui->m_fileTable->getFPFileInfos();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            m_fileProtectedProxy->RemoveFile(trustedInfo.filePath).waitForFinished();
        }
    }
    updateInfo();
}

}  // namespace KS
