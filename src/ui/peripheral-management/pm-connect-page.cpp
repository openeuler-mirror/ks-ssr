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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "src/ui/peripheral-management/pm-connect-page.h"
#include "src/ui/ui_pm-connect-page.h"
#include "src/ui/peripheral-management/edit-permissions.h"


#include <kiran-log/qt5-log-i.h>
#include <QPainter>
#include <QStyleOption>

namespace KS {

PMConnectPage::PMConnectPage(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::PMConnectPage)
{
    m_ui->setupUi(this);
    m_ui->m_title->setText(tr("Connect Record"));

    m_ui->m_search->addAction(QIcon(":/images/search"), QLineEdit::ActionPosition::LeadingPosition);
    connect(m_ui->m_search,&QLineEdit::textChanged,this,&PMConnectPage::searchTextChanged);
}

PMConnectPage::~PMConnectPage()
{
    delete m_ui;
}

void PMConnectPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void PMConnectPage::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    auto filterProxy = this->m_ui->m_table->getFilterProxy();
    filterProxy->setFilterFixedString(text);
}

void PMConnectPage::update()
{

}
}   //namespace KS
