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
#include <QLabel>
#include <QPainter>
#include <QStyleOption>

namespace KS
{
DatePickButton::DatePickButton(QWidget *parent)
    : QPushButton(parent), m_fristDate(nullptr), m_endDate(nullptr)
{
    initUI();
}

void DatePickButton::setStartDate(const QString &fristDate)
{
    m_fristDate->setText(fristDate);
}

void DatePickButton::setEndDate(const QString &endDate)
{
    m_endDate->setText(endDate);
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
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(10, 0, 10, 0);
    mainLayout->setSpacing(0);

    auto firstDateIcon = new QLabel(this);
    firstDateIcon->setObjectName("lab_icon");
    firstDateIcon->setFixedSize(16, 16);
    firstDateIcon->setPixmap(QPixmap(":/images/icon-calendar"));
    auto endDateIcon = new QLabel(this);
    endDateIcon->setObjectName("lab_icon");
    endDateIcon->setFixedSize(16, 16);
    endDateIcon->setPixmap(QPixmap(":/images/icon-calendar"));

    m_fristDate = new QPushButton(this);
    m_fristDate->setObjectName("m_fristDate");
    connect(m_fristDate, &QPushButton::clicked, this, &DatePickButton::fristDateClicked);

    auto labelSplit = new QLabel(this);
    labelSplit->setObjectName("labelSplit");
    labelSplit->setFixedSize(16, 16);
    labelSplit->setText("--");

    m_endDate = new QPushButton(this);
    m_endDate->setObjectName("m_fristDate");
    connect(m_endDate, &QPushButton::clicked, this, &DatePickButton::endDateClicked);

    mainLayout->addWidget(firstDateIcon);
    mainLayout->addWidget(m_fristDate);
    mainLayout->addWidget(labelSplit);
    mainLayout->addWidget(endDateIcon);
    mainLayout->addWidget(m_endDate);
}
}  // namespace KS
