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

#include "table.h"
#include "ui_table.h"
#include <QFileInfo>
#include <QSizePolicy>
#include <QVBoxLayout>

#define MAX_HEIGHT 1122
namespace KS
{
namespace BR
{
Table::Table(QWidget *parent,
             bool isOpenFilesScan,
             bool isScanVulnerability)
    : QWidget(parent),
      m_ui(new Ui::Table)
{
    m_ui->setupUi(this);

    if (isOpenFilesScan)
    {
        m_ui->m_columnName1->setText(tr("Scan Item"));
        m_ui->m_columnName1->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_ui->m_tableHeader->layout()->setContentsMargins(24, 9, 9, 9);

        m_ui->m_columnName2->setText(tr("Scan Type"));
        m_ui->m_columnName3->setText(tr("Remarks"));
        m_ui->m_columnName4->deleteLater();
        m_ui->m_tableHeader->layout()->itemAt(3)->widget()->setVisible(false);
    }
    else if (isScanVulnerability)
    {
        m_ui->m_columnName1->setText(tr("Scan rpm name"));
        m_ui->m_columnName1->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_ui->m_tableHeader->layout()->setContentsMargins(24, 9, 9, 9);

        m_ui->m_columnName2->setText(tr("Scan results"));
        m_ui->m_columnName3->setText(tr("Remarks"));
        m_ui->m_columnName4->deleteLater();
        m_ui->m_tableHeader->layout()->itemAt(3)->widget()->setVisible(false);
    }
    else
    {
        m_ui->m_columnName1->setText(tr("Test Item"));
        m_ui->m_columnName2->setText(tr("Before reinforcement result"));
        m_ui->m_columnName3->setText(tr("After reinforcement result"));
        m_ui->m_columnName4->setText(tr("Remarks"));
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
                    const QString &reinforceResult,
                    const QString &remarks,
                    const QColor &scanColor,
                    const QColor &reinforceColor,
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
    // 背景颜色需要判断，不在qss中设置
    auto style = QString("QWidget{background-color: %1;}").arg(backgroundColor);
    line->setStyleSheet(style);

    auto sizePolicy = line->sizePolicy();
    sizePolicy.setVerticalPolicy(QSizePolicy::Maximum);
    line->setSizePolicy(sizePolicy);
    line->setMaximumHeight(36);

    auto layout = new QHBoxLayout(line);
    layout->setContentsMargins(0, 0, 0, 0);

    auto nameLabel = new QLabel(line);
    nameLabel->setObjectName("nameLabel");
    auto scanLabel = new QLabel(line);
    auto reinforceLabel = new QLabel(line);
    auto remarksLabel = new QLabel(line);
    remarksLabel->setObjectName("remarksLabel");

    QPalette scanPe;
    scanPe.setColor(QPalette::WindowText, scanColor);

    QPalette reinforcePe;
    reinforcePe.setColor(QPalette::WindowText, reinforceColor);

    scanLabel->setPalette(scanPe);
    reinforceLabel->setPalette(reinforcePe);

    nameLabel->setFixedSize(240, 35);
    scanLabel->setFixedHeight(35);
    reinforceLabel->setFixedHeight(35);
    remarksLabel->setFixedHeight(35);

    scanLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    reinforceLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    remarksLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    nameLabel->setText("      " + name);
    scanLabel->setText(scanResult);
    reinforceLabel->setText(reinforceResult);
    remarksLabel->setText(remarks);

    layout->addWidget(nameLabel);
    layout->addWidget(scanLabel);
    layout->addWidget(reinforceLabel);
    layout->addWidget(remarksLabel);

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

    nameLabel->setFixedSize(240, 35);
    scanLabel->setFixedHeight(35);
    remarksLabel->setFixedHeight(35);

    scanLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    remarksLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    nameLabel->setText("      " + filesName);
    scanLabel->setText(scanType);
    remarksLabel->setText(remarks);

    layout->addWidget(nameLabel);
    layout->addWidget(scanLabel);
    layout->addWidget(remarksLabel);

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
