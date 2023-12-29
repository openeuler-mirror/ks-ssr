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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/ui/fp/file-protection-page.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include <QWidgetAction>
#include "config.h"
#include "src/ui/common/delete-notify.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/kss_dbus_proxy.h"
#include "src/ui/ui_file-protection-page.h"
#include "ssr-i.h"

namespace KS
{
namespace FP
{
FileProtectionPage::FileProtectionPage(QWidget *parent)
    : Page(parent),
      m_ui(new Ui::FileProtectionPage())
{
    m_ui->setupUi(this);

    m_fileProtectedProxy = new KSSDbusProxy(SSR_DBUS_NAME,
                                            SSR_KSS_INIT_DBUS_OBJECT_PATH,
                                            QDBusConnection::systemBus(),
                                            this);
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_fileTable->getFPFileInfos().size());
    m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setObjectName("searchButton");
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);

    connect(m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addProtectedFiles(bool)));
    //    connect(m_ui->m_update, SIGNAL(clicked(bool)), this, SLOT(updateClicked(bool)));
    connect(m_ui->m_unprotect, SIGNAL(clicked(bool)), this, SLOT(popDeleteNotify(bool)));
    connect(m_ui->m_fileTable, &FileTable::filesUpdate, this, &FileProtectionPage::updateTips);
}

FileProtectionPage::~FileProtectionPage()
{
    delete m_ui;
}

QString FileProtectionPage::getNavigationUID()
{
    return tr("File protected");
}

QString FileProtectionPage::getSidebarUID()
{
    return "";
}

QString FileProtectionPage::getSidebarIcon()
{
    return "";
}

QString FileProtectionPage::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_SECADM;
}

void FileProtectionPage::searchTextChanged(const QString &text)
{
    m_ui->m_fileTable->searchTextChanged(text);
}

void FileProtectionPage::addProtectedFiles(bool checked)
{
    RETURN_IF_TRUE(!checkTrustedLoadFinied(m_fileProtectedProxy->initialized()))
    auto fileNames = QFileDialog::getOpenFileNames(this, tr("Open file"), QDir::homePath());
    if (!fileNames.isEmpty())
    {
        auto reply = m_fileProtectedProxy->AddProtectedFiles(fileNames);
        CHECK_ERROR_FOR_DBUS_REPLY(reply);
        return;
    }
}

bool FileProtectionPage::checkTrustedLoadFinied(bool initialized)
{
    // 可信未初始化完成，不允许操作
    if (!initialized)
    {
        POPUP_MESSAGE_DIALOG(tr("Trusted data needs to be initialised,"
                                "please wait a few minutes before trying."));

        return false;
    }
    return true;
}

bool FileProtectionPage::isExistSelectedItem()
{
    auto trustedInfos = m_ui->m_fileTable->getFPFileInfos();
    for (auto trustedInfo : trustedInfos)
    {
        RETURN_VAL_IF_TRUE(trustedInfo.selected, true)
    }

    return false;
}

void FileProtectionPage::popDeleteNotify(bool checked)
{
    RETURN_IF_TRUE(!checkTrustedLoadFinied(m_fileProtectedProxy->initialized()))

    if (!isExistSelectedItem())
    {
        POPUP_MESSAGE_DIALOG(tr("Please select the content that needs to be removed."));
        return;
    }

    auto unprotectNotify = new DeleteNotify(this);
    unprotectNotify->setNotifyMessage(tr("Remove protection"), tr("The removal operation is irreversible."
                                                                  "Do you confirm the removal of the selected record from the whitelist?"));
    auto x = window()->x() + window()->width() / 2 - unprotectNotify->width() / 2;
    auto y = window()->y() + window()->height() / 2 - unprotectNotify->height() / 2;
    unprotectNotify->move(x, y);
    unprotectNotify->show();

    connect(unprotectNotify, &DeleteNotify::accepted, this, &FileProtectionPage::removeProtectedFiles);
}

void FileProtectionPage::removeProtectedFiles()
{
    QStringList fileList;
    auto trustedInfos = m_ui->m_fileTable->getFPFileInfos();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            fileList << trustedInfo.filePath;
        }
    }
    auto reply = m_fileProtectedProxy->RemoveProtectedFiles(fileList);
    CHECK_ERROR_FOR_DBUS_REPLY(reply)
}

void FileProtectionPage::updateTips(int total)
{
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(QString::number(total));
    m_ui->m_tips->setText(text);
}
}  // namespace FP
}  // namespace KS
