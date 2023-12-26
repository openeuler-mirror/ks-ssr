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
#include "calendar-widget.h"

#include <QLabel>
#include <QLayout>
#include <QLocale>
#include <QPainter>
#include <QProxyStyle>
#include <QPushButton>
#include <QStyleOption>
#include <QSize>
#include <QTableView>
#include <QTextCharFormat>

namespace KS
{
class QCustomStyle : public QProxyStyle
{
public:
    QCustomStyle(QWidget *parent)
    {
        setParent(parent);
    };

private:
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const
    {
        if (element == PE_FrameFocusRect)
        {
            return;
        }
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
};

CalendarWidget::CalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    m_selectableStart = minimumDate();
    m_selectableEnd = maximumDate();
    initControl();
}

CalendarWidget::~CalendarWidget()
{
}

QDate CalendarWidget::getSelectDate()
{
    return selectedDate();
}

void CalendarWidget::setSelectableStart(QDate start)
{
    m_selectableStart = start;
    updateCells();
}

void CalendarWidget::setSelectableEnd(QDate end)
{
    m_selectableEnd = end;
    updateCells();
}

QDate CalendarWidget::getSelectableStart()
{
    return m_selectableStart;
    updateCells();
}

QDate CalendarWidget::getSelectableEnd()
{
    return m_selectableEnd;
    updateCells();
}

void CalendarWidget::hideNextButton()
{
    m_rightMonthBtn->hide();
    m_rightYearBtn->hide();
}

void CalendarWidget::hidePreButton()
{
    m_leftMonthBtn->hide();
    m_leftYearBtn->hide();
}

void CalendarWidget::initControl()
{
    setObjectName("CalendarWidget");
    layout()->setSizeConstraint(QLayout::SetDefaultConstraint);
    setLocale(QLocale(QLocale::Chinese));
    setNavigationBarVisible(false);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    setStyle(new QCustomStyle(this));
    // TODO QSS
    this->setStyleSheet("background-color: #2d2d2d");

    QFont font;
    font.setPixelSize(14);
    QTextCharFormat singleFormat;
    singleFormat.setForeground(QColor(145, 145, 145));
    singleFormat.setBackground(QColor(57, 57, 57));
    singleFormat.setFont(font);

    QTextCharFormat weekFormat;
    weekFormat.setForeground(QColor(145, 145, 145));
    weekFormat.setFont(font);

    setHeaderTextFormat(singleFormat);
    setWeekdayTextFormat(Qt::Saturday, weekFormat);
    setWeekdayTextFormat(Qt::Sunday, weekFormat);

    initTopWidget();

    connect(this, &QCalendarWidget::currentPageChanged, [this](int year, int month) {
        setDataLabelTimeText(year, month);
    });
}

void CalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    QCalendarWidget::paintCell(painter, rect, date);
    if (date == selectedDate())
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(46, 179, 255));
        painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());
        painter->setPen(QColor(255, 255, 255));

        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
    }
    else if (date >= m_selectableStart && date <= m_selectableEnd)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(57, 57, 57));

        painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());
        painter->setPen(QColor(255, 255, 255));

        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
    }
    else if (date < m_selectableStart && date > m_selectableEnd)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(145, 145, 145));
        painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());

        painter->setPen(QColor(145, 145, 145));
        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
    }
}

void CalendarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CalendarWidget::initTopWidget()
{
    auto topWidget = new QWidget(this);
    topWidget->setObjectName("CalendarTopWidget");
    topWidget->setFixedHeight(40);
    topWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto hboxLayout = new QHBoxLayout(topWidget);
    hboxLayout->setContentsMargins(12, 0, 12, 0);
    hboxLayout->setSpacing(4);

    m_leftYearBtn = new QPushButton(this);
    m_leftMonthBtn = new QPushButton(this);
    m_rightYearBtn = new QPushButton(this);
    m_rightMonthBtn = new QPushButton(this);
    m_dataLabel = new QLabel(this);

    m_leftYearBtn->setIcon(QIcon(":/images/double-arrow-left"));
    m_leftYearBtn->setIconSize(QSize(12, 10));
    m_leftMonthBtn->setIcon(QIcon(":/images/arrow-left"));
    m_leftMonthBtn->setIconSize(QSize(8, 10));
    m_rightYearBtn->setIcon(QIcon(":/images/double-arrow-right"));
    m_rightYearBtn->setIconSize(QSize(12, 10));
    m_rightMonthBtn->setIcon(QIcon(":/images/right-arrow"));
    m_rightMonthBtn->setIconSize(QSize(8, 10));

    hboxLayout->addWidget(m_leftYearBtn);
    hboxLayout->addWidget(m_leftMonthBtn);
    hboxLayout->addStretch();
    hboxLayout->addWidget(m_dataLabel);
    hboxLayout->addStretch();
    hboxLayout->addWidget(m_rightMonthBtn);
    hboxLayout->addWidget(m_rightYearBtn);

    auto vBodyLayout = qobject_cast<QVBoxLayout *>(layout());
    vBodyLayout->insertWidget(0, topWidget);

    connect(m_leftYearBtn, SIGNAL(clicked()), this, SLOT(onbtnClicked()));
    connect(m_leftMonthBtn, SIGNAL(clicked()), this, SLOT(onbtnClicked()));
    connect(m_rightYearBtn, SIGNAL(clicked()), this, SLOT(onbtnClicked()));
    connect(m_rightMonthBtn, SIGNAL(clicked()), this, SLOT(onbtnClicked()));

    setDataLabelTimeText(selectedDate().year(), selectedDate().month());
}

void CalendarWidget::initBottomWidget()
{
    auto bottomWidget = new QWidget(this);
    bottomWidget->setObjectName("CalendarBottomWidget");
    bottomWidget->setFixedHeight(40);
    bottomWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto hboxLayout = new QHBoxLayout(bottomWidget);
    hboxLayout->setContentsMargins(12, 0, 12, 0);
    hboxLayout->setSpacing(6);

    m_ensureBtn = new QPushButton(this);
    m_ensureBtn->setObjectName("CalendarEnsureBtn");
    m_ensureBtn->setFixedSize(40, 22);
    m_ensureBtn->setText(tr("confirm"));

    m_toDayBtn = new QPushButton(this);
    m_toDayBtn->setObjectName("CalendarTodayBtn");
    m_toDayBtn->setFixedSize(40, 22);
    m_toDayBtn->setText(tr("now"));

    hboxLayout->addStretch();
    hboxLayout->addWidget(m_toDayBtn);
    hboxLayout->addWidget(m_ensureBtn);

    auto vBodyLayout = qobject_cast<QVBoxLayout *>(layout());
    vBodyLayout->addWidget(bottomWidget);

    connect(m_ensureBtn, &QPushButton::clicked, [this]() {
        emit signalSetCalendarTime(selectedDate());
        emit activated(selectedDate());
    });

    connect(m_toDayBtn, &QPushButton::clicked, [this]() {
        showToday();
    });
}

void CalendarWidget::setDataLabelTimeText(int year, int month)
{
    m_dataLabel->setText(QString(tr("%1year%2mouth")).arg(year).arg(month));
}

void CalendarWidget::onbtnClicked()
{
    auto senderBtn = qobject_cast<QPushButton *>(sender());
    if (senderBtn == m_leftYearBtn)
    {
        showPreviousYear();
    }
    else if (senderBtn == m_leftMonthBtn)
    {
        showPreviousMonth();
    }
    else if (senderBtn == m_rightYearBtn)
    {
        showNextYear();
    }
    else if (senderBtn == m_rightMonthBtn)
    {
        showNextMonth();
    }
}
}  // namespace KS
