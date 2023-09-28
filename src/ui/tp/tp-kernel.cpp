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
#include "src/ui/tp/tp-kernel.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include <QPainter>
#include <QWidgetAction>
#include "ksc-i.h"
#include "src/ui/common/ksc-marcos-ui.h"
#include "src/ui/kss_dbus_proxy.h"
#include "src/ui/tp/table-delete-notify.h"
#include "src/ui/ui_tp-kernel.h"

namespace KS
{
TPKernel::TPKernel(QWidget *parent) : QWidget(parent),
                                      m_ui(new Ui::TPKernel),
                                      m_dbusProxy(nullptr),
                                      m_refreshTimer(nullptr)
{
    m_ui->setupUi(this);

    m_dbusProxy = new KSSDbusProxy(KSC_DBUS_NAME,
                                   KSC_KSS_INIT_DBUS_OBJECT_PATH,
                                   QDBusConnection::systemBus(),
                                   this);
    // 初始化完成自动刷新
    connect(m_dbusProxy, &KSSDbusProxy::InitFinished, this, [this]
            { m_ui->m_kernelTable->updateInfo(); });
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2"))
                    .arg(QString::number(m_ui->m_kernelTable->getKernelRecords().size()),
                         QString::number(m_ui->m_kernelTable->getKerneltamperedNums()));
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
    connect(m_refreshTimer, &QTimer::timeout, this, &TPKernel::updateRefreshIcon);

    connect(m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addKernelFile(bool)));
    connect(m_ui->m_recertification, SIGNAL(clicked(bool)), this, SLOT(recertification(bool)));
    connect(m_ui->m_refresh, SIGNAL(clicked(bool)), this, SLOT(updateKernelList(bool)));
    connect(m_ui->m_unprotect, SIGNAL(clicked(bool)), this, SLOT(popDeleteNotify(bool)));
    connect(m_ui->m_kernelTable, SIGNAL(prohibitUnloadingStatusChange(bool, const QString &)), this, SLOT(prohibitUnloading(bool, const QString &)));
    connect(m_ui->m_kernelTable, SIGNAL(filesUpdate(int)), this, SLOT(updateTips(int)));
}

TPKernel::~TPKernel()
{
    delete m_ui;
}

void TPKernel::updateTips(int total)
{
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2"))
                    .arg(QString::number(total),
                         QString::number(m_ui->m_kernelTable->getKerneltamperedNums()));
    m_ui->m_tips->setText(text);
}

bool TPKernel::isExistSelectedItem()
{
    auto trustedInfos = m_ui->m_kernelTable->getKernelRecords();
    for (auto trustedInfo : trustedInfos)
    {
        RETURN_VAL_IF_TRUE(trustedInfo.selected, true)
    }

    return false;
}

void TPKernel::searchTextChanged(const QString &text)
{
    m_ui->m_kernelTable->searchTextChanged(text);
}

void TPKernel::addKernelFile(bool checked)
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath(), "", 0, QFileDialog::DontUseCustomDirectoryIcons);
    RETURN_IF_TRUE(fileName.isEmpty())

    auto reply = m_dbusProxy->AddTrustedFile(fileName);
    CHECK_ERROR_FOR_DBUS_REPLY(reply)
}

void TPKernel::updateKernelList(bool checked)
{
    m_refreshTimer->start(100);
    m_ui->m_kernelTable->updateInfo();
}

void TPKernel::recertification(bool checked)
{
    auto trustedInfos = m_ui->m_kernelTable->getKernelRecords();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            m_dbusProxy->AddTrustedFile(trustedInfo.filePath).waitForFinished();
        }
    }
}

void TPKernel::popDeleteNotify(bool checked)
{
    if (!isExistSelectedItem())
    {
        POPUP_MESSAGE_DIALOG(tr("Please select the content that needs to be removed."));
        return;
    }

    auto deleteNotify = new TableDeleteNotify(this);

    int x = window()->x() + width() / 4 + deleteNotify->width() / 4;
    int y = window()->y() + height() / 4 + deleteNotify->height() / 4;
    deleteNotify->move(x, y);
    deleteNotify->show();

    connect(deleteNotify, &TableDeleteNotify::accepted, this, &TPKernel::removeKernelFile);
}

void TPKernel::removeKernelFile()
{
    auto trustedInfos = m_ui->m_kernelTable->getKernelRecords();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            m_dbusProxy->RemoveTrustedFile(trustedInfo.filePath).waitForFinished();
        }
    }
}

void TPKernel::prohibitUnloading(bool status, const QString &path)
{
    RETURN_IF_TRUE(path.isEmpty())
    m_dbusProxy->ProhibitUnloading(status, path).waitForFinished();
}

void TPKernel::updateRefreshIcon()
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
