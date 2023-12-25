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

#include "access-control-page.h"
#include <QProcess>
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"
#include "src/ui/ui_access-control-page.h"

#define ACCESS_CONTROL_ICON_NAME "/images/access-control"

namespace KS
{
namespace ToolBox
{
AccessControlPage::AccessControlPage(QWidget* parent)
    : Page(parent),
      m_ui(new Ui::AccessControlPage)
{
    m_ui->setupUi(this);
    initUI();
}

AccessControlPage::~AccessControlPage()
{
    delete m_ui;
}

QString AccessControlPage::getNavigationUID()
{
    return tr("Tool Box");
}

QString AccessControlPage::getSidebarUID()
{
    return tr("Access Control");
}

QString AccessControlPage::getSidebarIcon()
{
    return ":" ACCESS_CONTROL_ICON_NAME;
}

QString AccessControlPage::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_SECADM;
}

void AccessControlPage::initUI()
{
    auto selinuxStatus = m_ui->m_table->getSelinuxStatus();
    m_ui->m_swich->setCheckState(selinuxStatus ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    connect(m_ui->m_swich, &QCheckBox::clicked, this, [this](bool checked) {
        RETURN_IF_TRUE(m_ui->m_table->openSelinux(checked));
        m_ui->m_swich->setCheckState(!checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    });
}
}  // namespace ToolBox
}  // namespace KS
