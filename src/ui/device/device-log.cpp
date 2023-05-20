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

#include "src/ui/device/device-log.h"
#include <kiran-log/qt5-log-i.h>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QWidgetAction>
#include "src/ui/device/device-permission.h"
#include "src/ui/device/table-filter-model.h"
#include "src/ui/ui_device-log.h"

namespace KS
{
DeviceLog::DeviceLog(QWidget *parent) : QWidget(parent),
                                        m_ui(new Ui::DeviceLog),
                                        m_deviceManagerProxy(nullptr)
{
    m_ui->setupUi(this);
    m_ui->m_title->setText(tr("Device Log"));

    //设置搜索框搜索图标
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);

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

void DeviceLog::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    auto filterProxy = m_ui->m_table->getFilterProxy();
    filterProxy->setFilterFixedString(text);
}

}  //namespace KS
