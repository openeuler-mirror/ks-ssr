/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "remote-page.h"
#include <QFileDialog>
#include "common/ssr-marcos-ui.h"
#include "src/ui/ui_remote-page.h"
#include "ssr-i.h"

#define STACK_PAGE_CONFIG 0
#define STACK_PAGE_PROGRESS 1

namespace KS
{
namespace RemotePage
{
RemotePage::RemotePage(QWidget* parent)
    : Page(parent),
      m_ui(new Ui::RemotePage),
      m_actuator(new ActuatorWrapper(this))
{
    m_ui->setupUi(this);
    m_ui->stackedWidget->setCurrentIndex(STACK_PAGE_CONFIG);

    connect(m_ui->btn_select, &QPushButton::clicked, this, &RemotePage::onSelectFile);
    connect(m_ui->btn_cancel, &QPushButton::clicked, this, &RemotePage::onCancel);
    connect(m_ui->btn_return, &QPushButton::clicked, this, &RemotePage::onReturnToConfig);
    connect(m_ui->btn_excute, &QPushButton::clicked, this, &RemotePage::onExecute);

    connect(m_actuator, &ActuatorWrapper::started,
            this, &RemotePage::onActuatorStarted);
    connect(m_actuator, &ActuatorWrapper::entryCompleted,
            this, &RemotePage::onActuatorEntryCompleted);
    connect(m_actuator, &ActuatorWrapper::finished,
            this, &RemotePage::onActuatorFinished);
    connect(m_actuator, &ActuatorWrapper::echoLog,
            this, &RemotePage::onActuatorEchoLog);
}

RemotePage::~RemotePage()
{
    delete m_ui;
}

QString RemotePage::getNavigationUID()
{
    return tr("Remote Manager");
}

QString RemotePage::getSidebarUID()
{
    return "";
}

QString RemotePage::getSidebarIcon()
{
    return "";
}

QString RemotePage::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_COMADM;
}

void RemotePage::onSelectFile()
{
    auto file = QFileDialog::getOpenFileName(this, tr("select machine login configuration"), qgetenv("HOME"), "all files(*)");
    m_ui->lineEdit->setText(file);
}

void RemotePage::onCancel()
{
    if (m_actuator->isRunning())
    {
        m_actuator->stop();
    }
}

void RemotePage::onExecute()
{
    QString filePath = m_ui->lineEdit->text();
    if (filePath.isEmpty())
    {
        POPUP_MESSAGE_DIALOG(tr("Please click the Select File button to select a machine login configuration"));
        return;
    }

    QFileInfo fileInfo(filePath);
    fileInfo.exists();
    if (!fileInfo.exists(filePath))
    {
        POPUP_MESSAGE_DIALOG(tr("Please reselect the file,%1 file not exists").arg(filePath));
        return;
    }

    if (!fileInfo.isReadable())
    {
        POPUP_MESSAGE_DIALOG(tr("Please reselect the file,%1 file can't readable").arg(filePath));
        return;
    }

    bool reinforce = m_ui->check_baseline->isChecked();
    bool repair = m_ui->check_repair->isChecked();
    if (!reinforce && !repair)
    {
        POPUP_MESSAGE_DIALOG(tr("Please select a batch operation"));
        return;
    }

    m_ui->textBrowser->clear();
    m_ui->btn_cancel->setVisible(true);
    m_ui->btn_return->setVisible(false);
    m_ui->progressBar->setValue(0);
    if (!m_actuator->start(filePath, reinforce, repair))
    {
        POPUP_MESSAGE_DIALOG(tr("Failed to start distribution"));
        return;
    }

    m_ui->stackedWidget->setCurrentIndex(STACK_PAGE_PROGRESS);
}

void RemotePage::onReturnToConfig()
{
    m_ui->stackedWidget->setCurrentIndex(STACK_PAGE_CONFIG);
}

void RemotePage::onActuatorStarted(int totalCount)
{
    m_ui->progressBar->setMaximum(totalCount);
    m_ui->progressBar->setValue(0);
}

void RemotePage::onActuatorEntryCompleted(int currentCount)
{
    m_ui->progressBar->setValue(currentCount);
}

void RemotePage::onActuatorFinished(bool isSuccessed, const QString& msg, const QString& logPath)
{
    m_ui->btn_cancel->setVisible(false);
    m_ui->btn_return->setVisible(true);
    POPUP_MESSAGE_DIALOG(msg);
}

void RemotePage::onActuatorEchoLog(const QString& log)
{
    m_ui->textBrowser->append(log);
}
}  // namespace RemotePage
}  // namespace KS