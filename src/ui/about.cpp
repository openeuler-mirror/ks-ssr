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
#include "about.h"
#include <QFile>
#include <QIcon>
#include "config-ui.h"
#include "ui_about.h"
namespace KS
{
#define VERSION_FILE_PATH KSC_INSTALL_DATADIR "/ks-sc.version"

About::About(QWidget *parent) : TitlebarWindow(parent),
                                m_ui(new Ui::about)
{
    m_ui->setupUi(getWindowContentWidget());

    initUI();
}

About::~About()
{
    delete m_ui;
}

void About::closeEvent(QCloseEvent *event)
{
}

void About::initUI()
{
    setAttribute(Qt::WA_DeleteOnClose);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setIcon(QIcon(":/images/logo"));
    setTitle(tr("About"));
    setTitleBarHeight(36);
    setFixedSize(429, 269);
    setWindowModality(Qt::ApplicationModal);
    setResizeable(false);

    m_ui->m_version->setText(tr("Security control sofware V1.0"));
    m_ui->m_info->setText(QString("ks-sc : %1").arg(getVersion(VERSION_FILE_PATH)));
    m_ui->m_license->setText("Copyright (c) 2023 ~ 2024 KylinSec Co. Ltd. All Rights Reserved.");
}

QString About::getVersion(const QString &filePath)
{
    QFile file(filePath);
    QString ret;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto ba = file.readLine();
        ret = QString(ba);

        file.close();
    }

    return ret.simplified();
}
}  // namespace KS