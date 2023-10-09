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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "src/ui/dm/device-log.h"
#include <kiran-log/qt5-log-i.h>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QWidgetAction>
#include "src/ui/dm/device-permission.h"
#include "src/ui/dm/table-filter-model.h"
#include "src/ui/ui_device-log.h"

namespace KS
{
namespace DM
{
DeviceLog::DeviceLog(QWidget *parent) : Page(parent),
                                        m_ui(new Ui::DeviceLog),
                                        m_deviceManagerProxy(nullptr)
{
    m_ui->setupUi(this);
    m_ui->m_title->setText(tr("Device Log"));

    //设置搜索框搜索图标
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setObjectName("searchButton");
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);

    m_deviceManagerProxy = new DeviceManagerProxy(SSR_DBUS_NAME,
                                                  SSR_DEVICE_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);
    connect(m_deviceManagerProxy, &DeviceManagerProxy::DeviceChanged, this, &DeviceLog::update);

    //获取设备记录数据插入表格
    update();

    connect(m_ui->m_search, &QLineEdit::textChanged, this, &DeviceLog::searchTextChanged);
}

DeviceLog::~DeviceLog()
{
    delete m_ui;
}

void DeviceLog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DeviceLog::update()
{
    m_ui->m_table->update();
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_table->getRowCount());
    m_ui->m_records->setText(text);
}

QString DeviceLog::getNavigationUID()
{
    return tr("Device management");
}

QString DeviceLog::getSidebarUID()
{
    return tr("Device Log");
}

QString DeviceLog::getSidebarIcon()
{
    return ":/images/device-log";
}

int DeviceLog::getSelinuxType()
{
    return 0;
}

void DeviceLog::searchTextChanged(const QString &text)
{
    auto filterProxy = m_ui->m_table->getFilterProxy();
    filterProxy->setFilterFixedString(text);
}

}  // namespace DM
}  // namespace KS
