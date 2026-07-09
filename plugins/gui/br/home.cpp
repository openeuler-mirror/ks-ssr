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
 * Author:     chendingjian <chendingjian@kylinsec.com.cn>
 */
#include "home.h"
#include <ssr-i.h>
#include <QDBusConnection>
#include <QDateTime>
#include <QMenu>
#include <QStyledItemDelegate>
#include "br-i.h"
#include "br_dbus_proxy.h"
#include "lib/widgets/ssr-marcos-ui.h"
#include "ui_home.h"

#define LOGO_PIXMAP_COUNTS 60

namespace KS
{
namespace BR
{
Home::Home(QWidget *parent)
    : QWidget(parent),
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
    m_logoTimer.stop();
    delete m_ui;
}

void Home::init()
{
    m_ui->m_reinforceTime->setText("");
    m_ui->m_scanButton->setText(BRStandardType(m_dbusProxy->strategy_type()) == BR_STANDARD_TYPE_SYSTEM ? tr("Quick scan") : tr("Custom scan"));
    m_ui->m_scanComboBox->setItemDelegate(new QStyledItemDelegate(this));
    m_ui->m_scanComboBox->addItems(QStringList() << tr("System strategy") << tr("Custom strategy"));
    m_ui->m_scanComboBox->setCurrentIndex(BRStandardType(m_dbusProxy->strategy_type()));
    connect(m_ui->m_scanComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int type)
            {
                auto reply = m_dbusProxy->SetStrategyType(type);
                CHECK_ERROR_FOR_DBUS_REPLY(reply);
                RETURN_IF_TRUE(reply.isError());
                m_ui->m_scanButton->setText(type == BR_STANDARD_TYPE_SYSTEM ? tr("Quick scan") : tr("Custom scan"));
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

    // 动画载入
    for (int i = 0; i <= LOGO_PIXMAP_COUNTS; i++)
    {
        QString res = QString(":/br/image/logo/%1").arg(i);
        m_logoPixVec.append(res);
    }

    connect(&m_logoTimer, &QTimer::timeout, this, [this]()
            {
                static uint pixIndex = 0;
                if (pixIndex > LOGO_PIXMAP_COUNTS)
                {
                    pixIndex = 0;
                }
                m_ui->m_icon->setPixmap(m_logoPixVec.at(pixIndex++));
            });
    m_logoTimer.start(12000 / LOGO_PIXMAP_COUNTS);
}

void Home::modfiyReinforcementTime()
{
    m_ui->m_reinforceTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

}  // namespace BR
}  // namespace KS
