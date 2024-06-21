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

#include <QFileInfo>
#include <QSizePolicy>
#include <QVBoxLayout>
#include "pdf-details.h"
#include "ui_pdf-details.h"

#define MAX_HEIGHT 1122
namespace KS
{
namespace BR
{
PDFDetails::PDFDetails(QWidget *parent,
             bool isOpenFilesScan,
             bool isScanVulnerability)
    : QWidget(parent),
      m_ui(new Ui::PDFDetails)
{
    m_ui->setupUi(this);
    m_rowHeight = 36;

    if (isOpenFilesScan)
    {
        m_ui->m_columnName1->setText(tr("Scan Item"));
        m_ui->m_columnName1->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_ui->m_columnName2->setText(tr("Scan Type"));
        m_ui->m_columnName2->setMinimumWidth(480);
        m_ui->m_tableHeader->layout()->itemAt(3)->widget()->setVisible(false);
    }
    else if (isScanVulnerability)
    {
        m_ui->m_columnName1->setText(tr("Scan rpm name"));
        m_ui->m_columnName1->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_ui->m_columnName2->setText(tr("Scan results"));
        m_ui->m_columnName2->setMinimumWidth(480);
        m_ui->m_tableHeader->layout()->itemAt(3)->widget()->setVisible(false);
    }
    else
    {
        m_ui->m_columnName1->setText(tr("Test Item"));
        m_ui->m_columnName2->setText(tr("Scan results"));
        m_ui->m_columnName1->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_ui->m_columnName2->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    }

    m_ui->m_tailBar->setText(tr("Technical support：Hunan KylinSec Technology Co. Ltd.,  Telephone：400-012-6606"));
    // 通过判断图片是否存在来确认字体颜色
    QFileInfo end(":/images/pdf-end");
    auto style = QString("QLabel{color: %1;}").arg(end.size() == 0 ? "black" : "white");
    m_ui->m_tailBar->setStyleSheet(style);
    m_ui->m_tailBar->hide();
    m_ui->m_tailPic->hide();
}

Table::~Table()
{
    delete m_ui;
}

void Table::addLine(const QString &name,
                    const QString &scanResult,
                    const QColor &scanColor,
                    const QString &backgroundColor)
{
    m_rowHeight += 36;
    m_ui->m_line->setMinimumHeight(m_rowHeight);
    if (m_rowHeight >= m_ui->m_page->height())
    {
        m_ui->m_page->setMinimumHeight(MAX_HEIGHT);
        m_ui->m_line->setMinimumHeight(MAX_HEIGHT - 100);
    }

    auto line = new QWidget(this);
    // 背景颜色需要判断，不在qss中设置
    auto style = QString("QWidget{background-color: %1;}").arg(backgroundColor);
    line->setStyleSheet(style);

    auto sizePolicy = line->sizePolicy();
    sizePolicy.setVerticalPolicy(QSizePolicy::Maximum);
    line->setSizePolicy(sizePolicy);
    line->setMaximumHeight(36);
    line->setMinimumHeight(36);

    auto layout = new QHBoxLayout(line);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(0);

    auto nameLabel = new QLabel(line);
    auto scanLabel = new QLabel(line);
    nameLabel->setObjectName("nameLabel");

    nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    scanLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    QPalette scanPe;
    scanPe.setColor(QPalette::WindowText, scanColor);
    scanLabel->setPalette(scanPe);

    nameLabel->setText(name);
    scanLabel->setText(scanResult);

    layout->addWidget(nameLabel);
    layout->addWidget(scanLabel);

    line->setLayout(layout);

    m_ui->m_line->layout()->addWidget(line);
    update();
}

void Table::addScanLine(const QString &filesName,
                        const QString &scanType,
                        const QString &remarks,
                        const QString &backgroundColor)
{
    m_rowHeight += 40;
    m_ui->m_line->setFixedHeight(m_rowHeight);
    if (m_rowHeight >= m_ui->m_page->height())
    {
        m_ui->m_page->setMinimumHeight(MAX_HEIGHT);
        m_ui->m_line->setMinimumHeight(MAX_HEIGHT - 100);
    }

    auto line = new QWidget(this);
    auto style = QString("QWidget{background-color: %1;}").arg(backgroundColor);
    line->setStyleSheet(style);
    line->setContentsMargins(16, 0, 0, 0);

    auto sizePolicy = line->sizePolicy();
    sizePolicy.setVerticalPolicy(QSizePolicy::Maximum);
    line->setSizePolicy(sizePolicy);
    line->setMaximumHeight(36);

    auto layout = new QHBoxLayout(line);
    layout->setContentsMargins(0, 0, 0, 0);

    auto nameLabel = new QLabel(line);
    nameLabel->setObjectName("nameLabel");
    auto scanLabel = new QLabel(line);
    scanLabel->setObjectName("scanLabel");
    auto remarksLabel = new QLabel(line);
    remarksLabel->setObjectName("remarksLabel");

    nameLabel->setFixedSize(220, 35);
    scanLabel->setFixedSize(280, 35);
    remarksLabel->setFixedSize(150, 35);

    scanLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    remarksLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    nameLabel->setText(filesName);
    scanLabel->setText(scanType);
    remarksLabel->setText(remarks);

    layout->addWidget(nameLabel);
    layout->addWidget(scanLabel);
    layout->addWidget(remarksLabel);
    layout->addStretch();

    line->setLayout(layout);

    m_ui->m_line->layout()->addWidget(line);
    update();
}

void Table::addSpacer()
{
    auto spacerWidget = new QWidget(this);
    spacerWidget->setObjectName("spacerWidget");
    spacerWidget->setMaximumHeight(1070);

    auto vlayout = new QVBoxLayout(spacerWidget);
    auto spacerItem = new QSpacerItem(758, 36, QSizePolicy::Expanding, QSizePolicy::Expanding);

    vlayout->addSpacerItem(spacerItem);
    spacerWidget->setLayout(vlayout);
    m_ui->m_line->layout()->addWidget(spacerWidget);
}

void Table::showTailBar()
{
    m_ui->m_tailBar->show();
    m_ui->m_tailPic->show();
}
}  // namespace BR
}  // namespace KS
