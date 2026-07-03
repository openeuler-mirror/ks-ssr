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

#include "privacy-cleanup-page.h"
#include <QWidgetAction>
#include "common/ssr-marcos-ui.h"
#include "include/ssr-i.h"
#include "src/ui/common/user-prompt-dialog.h"
#include "src/ui/ui_privacy-cleanup-page.h"

#define PRIVACY_CLEANUP_ICON_NAME "/images/privacy-cleanup"

namespace KS
{
namespace ToolBox
{
PrivacyCleanupPage::PrivacyCleanupPage(QWidget* parent)
    : Page(parent),
      m_ui(new Ui::PrivacyCleanupPage)
{
    m_ui->setupUi(this);
    initUI();
}
PrivacyCleanupPage::~PrivacyCleanupPage()
{
    delete m_ui;
}
QString PrivacyCleanupPage::getNavigationUID()
{
    return tr("Tool Box");
}
QString PrivacyCleanupPage::getSidebarUID()
{
    return tr("Privacy Cleanup");
}
QString PrivacyCleanupPage::getSidebarIcon()
{
    return ":" PRIVACY_CLEANUP_ICON_NAME;
}
QString PrivacyCleanupPage::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_SECADM;
}

void PrivacyCleanupPage::initUI()
{
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_table->getPrivacyCleanupInfosSize());
    m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setObjectName("searchButton");
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);

    connect(m_ui->m_search, &QLineEdit::textChanged, this, [this](const QString& text)
            {
                m_ui->m_table->setSearchText(text);
            });
    connect(m_ui->m_clean, &QPushButton::clicked, this, [this]
            {
                // 先检测是否有选中行
                if (m_ui->m_table->getCheckedUsers().isEmpty())
                {
                    POPUP_MESSAGE_DIALOG(tr("Please select items."));
                    return;
                }
                auto cleanNotify = new UserPromptDialog(this);
                cleanNotify->setNotifyMessage(tr("Privacy cleanup"), tr("The user privacy cleaning operation is irreversible. "
                                                                   "Are you sure you want to continue?"));
                auto x = window()->x() + window()->width() / 2 - cleanNotify->width() / 2;
                auto y = window()->y() + window()->height() / 2 - cleanNotify->height() / 2;
                cleanNotify->move(x, y);
                cleanNotify->show();
                connect(cleanNotify, &UserPromptDialog::accepted, this, [this]{
                    m_ui->m_table->cleanCheckedUsers();
                });
            });
    connect(m_ui->m_table, &PrivacyCleanupTable::tableUpdated, this, [this](int total)
            {
                // 更新表格右上角提示信息
                auto text = QString(tr("A total of %1 records")).arg(QString::number(total));
                m_ui->m_tips->setText(text);
            });
}
}  // namespace ToolBox
}  // namespace KS
