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
#include "src/ui/tp/execute-protected-page.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include <QPainter>
#include <QWidgetAction>
#include "src/ui/common/delete-notify.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/kss_dbus_proxy.h"
#include "src/ui/ui_execute-protected-page.h"
#include "ssr-i.h"

namespace KS
{
namespace TP
{
ExecuteProtectedPage::ExecuteProtectedPage(QWidget *parent) : Page(parent),
                                                              m_ui(new Ui::ExecuteProtectedPage),
                                                              m_dbusProxy(nullptr),
                                                              m_refreshTimer(nullptr)
{
    m_ui->setupUi(this);
    m_ui->m_note->setWordWrap(true);
    m_dbusProxy = new KSSDbusProxy(SSR_DBUS_NAME,
                                   SSR_KSS_INIT_DBUS_OBJECT_PATH,
                                   QDBusConnection::systemBus(),
                                   this);

    // 初始化完成自动刷新
    connect(m_dbusProxy, &KSSDbusProxy::InitFinished, this, [this] {
        m_ui->m_executeTable->updateInfo();
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
    connect(m_refreshTimer, &QTimer::timeout, this, &ExecuteProtectedPage::updateRefreshIcon);

    connect(m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addExecuteFile(bool)));
    connect(m_ui->m_recertification, SIGNAL(clicked(bool)), this, SLOT(recertification(bool)));
    connect(m_ui->m_refresh, SIGNAL(clicked(bool)), this, SLOT(updateExecuteList(bool)));
    connect(m_ui->m_unprotect, SIGNAL(clicked(bool)), this, SLOT(popDeleteNotify(bool)));
    // 防止有重名信号干扰，不使用SIGNAL-SLOT宏
    connect(m_ui->m_executeTable, &ExecuteProtectedTable::filesUpdate, this, &ExecuteProtectedPage::updateTips);
}

ExecuteProtectedPage::~ExecuteProtectedPage()
{
    delete m_ui;
}

bool ExecuteProtectedPage::getInitialized()
{
    return m_dbusProxy->initialized();
}

QString ExecuteProtectedPage::getNavigationUID()
{
    return tr("Trusted protected");
}

QString ExecuteProtectedPage::getSidebarUID()
{
    return tr("Execute protecked");
}

QString ExecuteProtectedPage::getSidebarIcon()
{
    return ":/images/execution-control";
}

QString ExecuteProtectedPage::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_SECADM;
}

void ExecuteProtectedPage::updateTips(int total)
{
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records, Being tampered with %2"))
                    .arg(QString::number(total),
                         QString::number(m_ui->m_executeTable->getExecutetamperedNums()));
    m_ui->m_tips->setText(text);
}

bool ExecuteProtectedPage::isExistSelectedItem()
{
    auto trustedInfos = m_ui->m_executeTable->getExecuteRecords();
    for (auto trustedInfo : trustedInfos)
    {
        RETURN_VAL_IF_TRUE(trustedInfo.selected, true)
    }

    return false;
}

void ExecuteProtectedPage::searchTextChanged(const QString &text)
{
    m_ui->m_executeTable->setSearchText(text);
}

void ExecuteProtectedPage::addExecuteFile(bool checked)
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath());
    RETURN_IF_TRUE(fileName.isEmpty())

    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix() == "ko" || fileInfo.suffix() == "ko.xz")
    {
        POPUP_MESSAGE_DIALOG(tr("Added file types are not supported."));
        return;
    }

    auto reply = m_dbusProxy->AddTrustedFile(fileName);
    CHECK_ERROR_FOR_DBUS_REPLY(reply)
}

void ExecuteProtectedPage::updateExecuteList(bool checked)
{
    m_refreshTimer->start(100);
    m_ui->m_executeTable->updateInfo();
}

void ExecuteProtectedPage::recertification(bool checked)
{
    QStringList fileList;
    auto trustedInfos = m_ui->m_executeTable->getExecuteRecords();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            fileList << trustedInfo.filePath;
        }
    }
    auto reply = m_dbusProxy->AddTrustedFiles(fileList);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
}

void ExecuteProtectedPage::popDeleteNotify(bool checked)
{
    if (!isExistSelectedItem())
    {
        POPUP_MESSAGE_DIALOG(tr("Please select the content that needs to be removed."));
        return;
    }

    auto deleteNotify = new DeleteNotify(this);
    deleteNotify->setNotifyMessage(tr("Remove protection"), tr("The removal operation is irreversible."
                                                               "Do you confirm the removal of the selected record from the whitelist?"));
    auto x = window()->x() + window()->width() / 2 - deleteNotify->width() / 2;
    auto y = window()->y() + window()->height() / 2 - deleteNotify->height() / 2;

    deleteNotify->move(x, y);
    deleteNotify->show();

    connect(deleteNotify, &DeleteNotify::accepted, this, &ExecuteProtectedPage::removeExecuteFiles);
}

void ExecuteProtectedPage::removeExecuteFiles()
{
    QStringList fileList;
    auto trustedInfos = m_ui->m_executeTable->getExecuteRecords();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            fileList << trustedInfo.filePath;
        }
    }
    auto reply = m_dbusProxy->RemoveTrustedFiles(fileList);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
}

void ExecuteProtectedPage::updateRefreshIcon()
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

}  // namespace TP
}  // namespace KS
