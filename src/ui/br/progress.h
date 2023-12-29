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
#pragma once

#include <QWidget>

namespace Ui
{
class Progress;
}

namespace KS
{
namespace BR
{
enum ProcessMethod
{
    PROCESS_METHOD_SCAN,
    PROCESS_METHOD_FASTEN
};

struct ProgressInfo
{
    int jobID;
    int jobState;
    int total = 0;
    int successCount = 0;
    int failureCount = 0;
    double progress;
    ProcessMethod method = PROCESS_METHOD_SCAN;
};

class Progress : public QWidget
{
    Q_OBJECT

public:
    explicit Progress(QWidget *parent = nullptr);
    virtual ~Progress();

    void updateProgressUI(ProcessMethod method);
    void updateProgress(ProgressInfo info);
    void resetProgress();

private:
    // 初始化进程时间
    void timeInit();
    void completeProcess(ProgressInfo info);

signals:
    void scanClicked();
    void reinforcementClicked();
    void returnHomeClicked();
    void generateReportClicked();
    // TODO : 等待超时信号，暂未使用
    void waitTimeOut();
    void cancelClicked();

private slots:
    void stopWorkingProcess();
    void changeProgress();
    void waitProgress();

private:
    Ui::Progress *m_ui;
    // 进程定时器，用于定时刷新进程进度
    QTimer *m_progressTimer;
    // 用于记录取消加固/扫描的时长，设置120s发出超时信号
    QTimer *m_waitTimer;
    int m_waitTimeOut = 0;
    // 进程使用的总时长
    int m_secTime;
};
}  // namespace BR
}  // namespace KS
