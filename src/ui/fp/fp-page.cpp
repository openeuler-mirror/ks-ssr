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
#include <QWidgetAction>
#include "config.h"
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "src/ui/common/message-dialog.h"
#include "src/ui/kss_dbus_proxy.h"
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

    m_fileProtectedProxy = new KSSDbusProxy(KSC_DBUS_NAME,
                                            KSC_KSS_INIT_DBUS_OBJECT_PATH,
                                            QDBusConnection::systemBus(),
                                            this);
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_fileTable->getFPFileInfos().size());
    m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setIcon(QIcon(":/images/search"));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);

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
    RETURN_IF_TRUE(!checkTrustedLoadFinied(m_fileProtectedProxy->initialized()))
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath());
    if (!fileName.isEmpty())
    {
        m_fileProtectedProxy->AddFPFile(fileName).waitForFinished();
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

bool FPPage::checkTrustedLoadFinied(int initialized)
{
    // 可信未初始化完成，不允许操作
    if (initialized == 0)
    {
        auto messgeDialog = new MessageDialog(this);
        messgeDialog->setMessage(tr("Trusted data needs to be initialised,"
                                    "please wait a few minutes before trying."));

        int x = this->x() + width() / 4 + messgeDialog->width() / 4;
        int y = this->y() + height() / 4 + messgeDialog->height() / 4;
        messgeDialog->move(x, y);
        messgeDialog->show();
        return false;
    }
    return true;
}

void FPPage::unprotectClicked(bool checked)
{
    RETURN_IF_TRUE(!checkTrustedLoadFinied(m_fileProtectedProxy->initialized()))
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
            m_fileProtectedProxy->RemoveFPFile(trustedInfo.filePath).waitForFinished();
        }
    }
    updateInfo();
}

}  // namespace KS
