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
#include "date-pick-button.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>

namespace KS
{
DatePickButton::DatePickButton(QWidget *parent) : QPushButton(parent), m_dateLabel(nullptr)
{
    initUI();
}

void DatePickButton::setText(const QString &date)
{
    m_dateLabel->setText(date);
}

void DatePickButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DatePickButton::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(10, 0, 0, 0);
    mainLayout->setSpacing(10);
    setLayout(mainLayout);

    QLabel *labIcon = new QLabel(this);
    labIcon->setObjectName("lab_icon");
    labIcon->setFixedSize(16, 16);
    labIcon->setPixmap(QPixmap(":/images/icon-calendar"));

    m_dateLabel = new QLabel(this);

    mainLayout->addWidget(labIcon);
    mainLayout->addWidget(m_dateLabel);
}
}  // namespace KS
