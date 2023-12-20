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
#include "progress.h"
#include <QDateTime>
#include <QTimer>
#include "include/ssr-marcos.h"
#include "src/ui/br/progress-icon.h"
#include "ui_progress.h"

namespace KS
{
namespace BR
{
Progress::Progress(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::Progress)
{
    m_ui->setupUi(this);
    setMouseTracking(true);

    timeInit();
    resetProgress();
}

Progress::~Progress()
{
    delete m_ui;
}

void Progress::resetProgress()
{
    m_ui->m_icon->finishedProgress(ProgressIconStatus::PROGRESS_ICON_STATUS_INITIAL);
    m_progressTimer->stop();
    m_ui->m_definition->setText(tr("Security reinforcement is protecting your computer"));
    m_ui->m_note->setText(tr("KylinSec Host Security Reinforcement Software Detects Risks in Advance to Ensure Asset Security"));
    m_ui->m_scan->disconnect();
    m_ui->m_scan->setText(tr("Strat scan"));
    connect(m_ui->m_scan, &QPushButton::clicked, this, &Progress::scanClicked);

    m_ui->m_return->disconnect();
    m_ui->m_return->setText(tr("Return"));
    connect(m_ui->m_return, &QPushButton::clicked, this, &Progress::returnHomeClicked);
    // 自定义扫描页面首页无需进度条和导出报表
    m_ui->m_progressBar->hide();
    m_ui->m_generateReport->hide();
    m_ui->m_scan->show();
    m_ui->m_return->show();
    m_ui->m_generateReport->disconnect();
    connect(m_ui->m_generateReport, &QPushButton::clicked, this, &Progress::generateReportClicked);
}

void Progress::timeInit()
{
    m_progressTimer = new QTimer(this);
    m_waitTimer = new QTimer(this);
    m_waitTimer->stop();
    m_secTime = 0;
    connect(m_progressTimer, SIGNAL(timeout()), this, SLOT(changeProgress()));
    connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(waitProgress()));
}

void Progress::updateProgressUI(ProcessMethod method)
{
    m_secTime = 0;
    m_progressTimer->start(1000);
    if (method == PROCESS_METHOD_FASTEN)
    {
        m_ui->m_generateReport->hide();
    }
    m_ui->m_icon->finishedProgress(ProgressIconStatus::PROGRESS_ICON_STATUS_WORKING);
    m_ui->m_definition->setText(QString(tr("In %1, please wait..."))
                                    .arg(method == PROCESS_METHOD_SCAN
                                             ? QString(tr("Scan"))
                                             : QString(tr("Reinforcement"))));
    m_ui->m_note->setText(tr("Start time: %1 elapsed time: 00:00:00 progress: 0%")
                              .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));

    m_ui->m_return->disconnect();
    m_ui->m_return->setText(tr("Cancel"));
    connect(m_ui->m_return, &QPushButton::clicked, this, &Progress::stopWorkingProcess);
    m_ui->m_progressBar->show();

    m_ui->m_scan->hide();
    m_ui->m_scan->disconnect();
}

void Progress::updateProgress(ProgressInfo info)
{
    m_ui->m_progressBar->setValue(info.progress);
    QTime time(0, 0, 0);
    auto useTime = time.addSecs(m_secTime).toString("hh:mm:ss");
    m_ui->m_note->setText(tr("Start time: %1 elapsed time: %2 progress: %3%")
                              .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"))
                              .arg(useTime)
                              .arg(info.progress));
    m_ui->m_progressBar->show();
    update();
    completeProcess(info);
}

void Progress::completeProcess(ProgressInfo info)
{
    RETURN_IF_TRUE(info.progress != double(100));

    if (info.successCount == info.total)
    {
        m_ui->m_definition->setText(info.method == PROCESS_METHOD_SCAN
                                        ? QString(tr("Scanned %1, %2 conform!")).arg(info.total).arg(info.successCount)
                                        : QString(tr("Reinforcement completed %1, successfully reinforced %2!"))
                                              .arg(info.total)
                                              .arg(info.successCount));
    }
    else
    {
        m_ui->m_definition->setText(info.method == PROCESS_METHOD_SCAN
                                        ? QString(tr("Scanned %1, %2 conform, %3 inconform!"))
                                              .arg(info.successCount + info.failureCount)
                                              .arg(info.successCount)
                                              .arg(info.failureCount)
                                        : QString(tr("Reinforcement completed %1, successfully reinforced %2, failed %3!"))
                                              .arg(info.successCount + info.failureCount)
                                              .arg(info.successCount)
                                              .arg(info.failureCount));
    }
    m_progressTimer->stop();
    m_ui->m_progressBar->hide();
    m_ui->m_return->disconnect();
    m_ui->m_scan->disconnect();
    m_ui->m_scan->show();
    m_ui->m_return->show();
    m_ui->m_icon->finishedProgress(ProgressIconStatus::PROGRESS_ICON_STATUS_FINISHED);

    if (info.method == PROCESS_METHOD_SCAN)
    {
        m_ui->m_scan->setText(tr("Reinforcement"));
        m_ui->m_return->setText(tr("Return"));
        m_ui->m_generateReport->show();
        connect(m_ui->m_return, &QPushButton::clicked, this, &Progress::returnHomeClicked);
        connect(m_ui->m_scan, &QPushButton::clicked, this, &Progress::reinforcementClicked);
    }
    else
    {
        m_ui->m_scan->setText(tr("GenerateReport"));
        m_ui->m_return->setText(tr("Return"));
        connect(m_ui->m_scan, &QPushButton::clicked, this, &Progress::generateReportClicked);
        connect(m_ui->m_return, &QPushButton::clicked, this, &Progress::returnHomeClicked);
    }
}  // namespace BR

void Progress::stopWorkingProcess()
{
    m_ui->m_note->setText(QString(tr("Start time: %1 elapsed time: %2"))
                              .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"))
                              .arg(QTime(0, 0, 0).toString("hh:mm:ss")));
    m_progressTimer->stop();
    m_waitTimer->setInterval(0);
    m_waitTimer->start(1000);
    emit cancelClicked();
}

void Progress::changeProgress()
{
    auto current_time = QDateTime::currentDateTime();
    m_secTime++;
    auto strDate = current_time.toString("yyyy-MM-dd");
    QTime time(0, 0, 0);
    auto useTime = time.addSecs(m_secTime).toString("hh:mm:ss");
    m_ui->m_note->setText(QString(tr("Start time: %1 elapsed time: %2 progress: %3%"))
                              .arg(strDate)
                              .arg(useTime)
                              .arg(m_ui->m_progressBar->value()));
}

void Progress::waitProgress()
{
    ++m_waitTimeOut;
    RETURN_IF_TRUE(m_waitTimeOut <= 120)
    m_waitTimeOut = 0;
    m_waitTimer->stop();
    emit waitTimeOut();
}
}  // namespace BR
}  // namespace KS
