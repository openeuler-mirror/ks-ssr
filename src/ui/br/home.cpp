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
#include "home.h"
#include <QDBusConnection>
#include <QDateTime>
#include <QStyledItemDelegate>
#include <QMenu>
#include "include/ssr-i.h"
#include "src/ui/br/br-i.h"
#include "src/ui/br_dbus_proxy.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "ui_home.h"

namespace KS
{
namespace BR
{
Home::Home(QWidget *parent) : QWidget(parent),
                              m_ui(new Ui::Home)
{
    m_ui->setupUi(this);

    m_dbusProxy = new BRDbusProxy(SSR_DBUS_NAME,
                                  BR_DBUS_OBJECT_PATH,
                                  QDBusConnection::systemBus(),
                                  this);

    init();
}

Home::~Home()
{
    delete m_ui;
}

void Home::init()
{
    m_ui->m_reinforceTime->setText("");
    m_ui->m_icon->setPixmap(QPixmap(":/images/br-banner"));
    m_ui->m_scanComboBox->setItemDelegate(new QStyledItemDelegate(this));
    m_ui->m_scanComboBox->addItems(QStringList() << tr("System strategy") << tr("Custom strategy"));
    m_ui->m_scanComboBox->setCurrentIndex(BRStandardType(m_dbusProxy->strategy_type()));
    connect(m_ui->m_scanComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int type)
            {
                auto reply = m_dbusProxy->SetStrategyType(type);
                CHECK_ERROR_FOR_DBUS_REPLY(reply)
                emit currentStrategyChanged(type);
            });
    connect(m_ui->m_scanButton, &QPushButton::clicked, this, [this]()
            {
                if (m_ui->m_scanComboBox->currentText() == tr("Custom strategy"))
                {
                    emit customScanClicked();
                }
                else if (m_ui->m_scanComboBox->currentText() == tr("System strategy"))
                {
                    emit systemScanClicked();
                }
            });
}

void Home::modfiyReinforcementTime()
{
    m_ui->m_reinforceTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

}  // namespace BR
}  // namespace KS
