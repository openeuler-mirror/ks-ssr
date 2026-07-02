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

#include "src/ui/peripheral-management/pm-device-page.h"
#include "src/ui/ui_pm-device-page.h"
#include "src/ui/peripheral-management/edit-permissions.h"


#include <kiran-log/qt5-log-i.h>
#include <QPainter>

namespace KS {

PMDevicePage::PMDevicePage(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::PMDevicePage),
    m_editPermission(nullptr)
{
    m_ui->setupUi(this);
    m_ui->m_title->setText(tr("Device List"));

    m_ui->m_search->addAction(QIcon(":/images/search"), QLineEdit::ActionPosition::LeadingPosition);
    connect(m_ui->m_search,&QLineEdit::textChanged,this,&PMDevicePage::searchTextChanged);
    connect(m_ui->m_edit,&QPushButton::clicked,this,&PMDevicePage::editClicked);
}

PMDevicePage::~PMDevicePage()
{
    delete m_ui;
    if(m_editPermission)
    {
        delete m_editPermission;
        m_editPermission = nullptr;
    }

}

void PMDevicePage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void PMDevicePage::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    auto filterProxy = this->m_ui->m_table->getFilterProxy();
    filterProxy->setFilterFixedString(text);
}

void PMDevicePage::editClicked(bool checked)
{
    if(m_editPermission)
    {
       m_editPermission->show();
       return;
    }

    m_editPermission = new EditPermissions();
    connect(m_editPermission,&EditPermissions::permissionChanged,this,&PMDevicePage::update);

    int x = this->x() + this->width() / 4 + m_editPermission->width() / 4;
    int y = this->y() + this->height() / 4 + m_editPermission->height() / 4;
    this->m_editPermission->move(x, y);
    this->m_editPermission->show();
}

void PMDevicePage::update()
{

}
}   //namespace KS
