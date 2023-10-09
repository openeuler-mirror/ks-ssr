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

#include "loading.h"
#include <kiran-log/qt5-log-i.h>
#include <QGridLayout>
#include <QMouseEvent>
#include <QMovie>

namespace KS
{
#define LOADING_PIXMAP_COUNTS 18
static int pixmapIndex = 1;

Loading::Loading(QWidget *parent) : QWidget(parent), m_labLoading(nullptr)
{
    initUI();

    m_timer = new QTimer(this);
    m_timer->setInterval(50);
    connect(m_timer, &QTimer::timeout, this, &Loading::updatePixmap);
    m_timer->start();
    setAutoFillBackground(true);
}

Loading::~Loading()
{
}

void Loading::initUI()
{
    pixmapIndex = 1;
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setObjectName("maskWidget");

    auto vLayout = new QVBoxLayout(this);

    m_labLoading = new QLabel(this);
    m_labLoading->setPixmap(QPixmap(":/images/loading-18"));
    m_labLoading->setAlignment(Qt::AlignCenter);

    auto label = new QLabel(tr("The data is being initialized, please wait a moment."), this);
    label->setObjectName("loadingLabel");
    label->setAlignment(Qt::AlignCenter);

    vLayout->addSpacing(175);
    vLayout->addWidget(m_labLoading);
    vLayout->addWidget(label);
    vLayout->addStretch();

    hide();
}

void Loading::updatePixmap()
{
    if (pixmapIndex > LOADING_PIXMAP_COUNTS)
    {
        pixmapIndex = 2;
    }

    m_labLoading->setPixmap(QPixmap(QString(":/images/loading-%1").arg(QString::number(pixmapIndex))));
    pixmapIndex++;
}
}  // namespace KS
