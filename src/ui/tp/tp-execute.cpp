/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
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
#include "src/ui/tp/tp-execute.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include <QWidgetAction>
#include <QPainter>
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "src/ui/common/message-dialog.h"
#include "src/ui/kss_dbus_proxy.h"
#include "src/ui/tp/table-delete-notify.h"
//#include "src/ui/"
#include "src/ui/ui_tp-execute.h"

namespace KS
{
TPExecute::TPExecute(QWidget *parent) : QWidget(parent),
                                        m_ui(new Ui::TPExecute),
                                        m_dbusProxy(nullptr),
                                        m_refreshTimer(nullptr)
{
    m_ui->setupUi(this);
    m_ui->m_note->setWordWrap(true);
    m_dbusProxy = new KSSDbusProxy(KSC_DBUS_NAME,
                                   KSC_KSS_INIT_DBUS_OBJECT_PATH,
                                   QDBusConnection::systemBus(),
                                   this);

    // 初始化完成自动刷新
    connect(m_dbusProxy, &KSSDbusProxy::InitFinished, this, [this]
            {
                updateInfo();
                emit initFinished();
            });
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2"))
            .arg(QString::number(m_ui->m_executeTable->getExecuteRecords().size()),
                 QString::number(m_ui->m_executeTable->getExecutetamperedNums()));
    m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setObjectName("searchButton");
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);

    // 刷新
    m_ui->m_refresh->setIcon(QIcon(":/images/refresh"));
    m_ui->m_refresh->setIconSize(QSize(16, 16));
    m_ui->m_refresh->installEventFilter(this);

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &TPExecute::refreshTimerTimeout);

    connect(m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addClicked(bool)));
    connect(m_ui->m_recertification, SIGNAL(clicked(bool)), this, SLOT(recertificationClicked(bool)));
    connect(m_ui->m_refresh, SIGNAL(clicked(bool)), this, SLOT(refreshClicked(bool)));
    connect(m_ui->m_unprotect, SIGNAL(clicked(bool)), this, SLOT(unprotectClicked(bool)));
}

TPExecute::~TPExecute()
{
    delete m_ui;
}

bool TPExecute::getInitialized()
{
    return m_dbusProxy->initialized();
}

void TPExecute::updateInfo()
{
    m_ui->m_executeTable->updateRecord();
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2"))
            .arg(QString::number(m_ui->m_executeTable->getExecuteRecords().size()),
                 QString::number(m_ui->m_executeTable->getExecutetamperedNums()));
    m_ui->m_tips->setText(text);
}

void TPExecute::searchTextChanged(const QString &text)
{
    m_ui->m_executeTable->searchTextChanged(text);
}

void TPExecute::addClicked(bool checked)
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath());
    RETURN_IF_TRUE(fileName.isEmpty())

    auto reply = m_dbusProxy->AddTrustedFile(fileName);
    reply.waitForFinished();

    if (reply.isError())
    {
        KLOG_WARNING() << "Failed to add files: " << reply.error().message();
        auto messgeDialog = new MessageDialog(this);
        messgeDialog->setMessage(reply.error().message());

        int x = this->x() + this->width() / 4 + messgeDialog->width() / 4;
        int y = this->y() + this->height() / 4 + messgeDialog->height() / 4;
        messgeDialog->move(x, y);
        messgeDialog->show();
    }
    updateInfo();
}

void TPExecute::refreshClicked(bool checked)
{
    m_refreshTimer->start(100);
    updateInfo();
}

void TPExecute::recertificationClicked(bool checked)
{
    auto trustedInfos = m_ui->m_executeTable->getExecuteRecords();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            m_dbusProxy->AddTrustedFile(trustedInfo.filePath).waitForFinished();
        }
    }
    updateInfo();
}

void TPExecute::unprotectClicked(bool checked)
{
    auto unprotectNotify = new TableDeleteNotify(this);
    unprotectNotify->show();

    connect(unprotectNotify, &TableDeleteNotify::accepted, this, &TPExecute::unprotectAccepted);
}

void TPExecute::unprotectAccepted()
{
    auto trustedInfos = m_ui->m_executeTable->getExecuteRecords();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            m_dbusProxy->RemoveTrustedFile(trustedInfo.filePath).waitForFinished();
        }
    }
    updateInfo();
}

void TPExecute::refreshTimerTimeout()
{
    static int count = 0;
    count++;
    QPixmap pix(":/images/refresh-hover");
    static int rat = 0;
    rat = rat >= 180 ? 30 : rat + 30;
    int imageWidth = pix.width();
    int imageHeight = pix.height();
    QPixmap temp(pix.size());
    temp.fill(Qt::transparent);
    QPainter painter(&temp);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.translate(imageWidth / 2, imageHeight / 2);        //让图片的中心作为旋转的中心
    painter.rotate(rat);                                       //顺时针旋转90度
    painter.translate(-(imageWidth / 2), -(imageHeight / 2));  //使原点复原
    painter.drawPixmap(0, 0, pix);
    painter.end();
    m_ui->m_refresh->setIcon(QIcon(temp));

    if (count == 6)
    {
        m_refreshTimer->stop();
        m_ui->m_refresh->setIcon(QIcon(":/images/refresh"));
        count = 0;
    }
}

}  // namespace KS
