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

#include "pdf.h"
#include "round-progressbar.h"
#include "ui_pdf.h"

namespace KS
{
namespace BR
{
PDF::PDF(const QString &systemName,
         const QString &IP,
         const QString &MAC,
         const QString &kernel,
         const QString &activeStatus,
         QWidget *parent) : QWidget(parent),
                            m_ui(new Ui::PDF)
{
    m_ui->setupUi(this);

    m_ui->m_systemName->setText(systemName);
    m_ui->m_IP->setText(IP);
    m_ui->m_MAC->setText(MAC);
    m_ui->m_kernel->setText(kernel);
    m_ui->m_activeStatus->setText(activeStatus);
}

PDF::~PDF()
{
    delete m_ui;
}

void PDF::setPieChartText(const QString name[],
                          int total[],
                          int conform[],
                          int inconform[])
{
    m_pieChart1 = new RoundProgressBar(name[0], total[0], conform[0], inconform[0], this);
    m_pieChart2 = new RoundProgressBar(name[1], total[1], conform[1], inconform[1], this);
    m_pieChart3 = new RoundProgressBar(name[2], total[2], conform[2], inconform[2], this);
    m_pieChart4 = new RoundProgressBar(name[3], total[3], conform[3], inconform[3], this);

    m_ui->m_pieChartLayout->addWidget(m_pieChart1);
    m_ui->m_pieChartLayout->addWidget(m_pieChart2);

    m_ui->m_pieChartLayout2->addWidget(m_pieChart3);
    m_ui->m_pieChartLayout2->addWidget(m_pieChart4);
}
}  // namespace BR
}  // namespace KS
