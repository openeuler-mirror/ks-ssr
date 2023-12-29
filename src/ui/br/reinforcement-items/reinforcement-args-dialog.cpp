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

#include "reinforcement-args-dialog.h"
#include <qt5-log-i.h>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QHeaderView>
#include "include/ssr-marcos.h"
#include "ui_reinforcement-args-dialog.h"

namespace KS
{
namespace BR
{
ReinforcementArgsDialog::ReinforcementArgsDialog(QWidget *parent)
    : TitlebarWindow(parent),
      m_currentArgHandle(nullptr),
      m_ui(new Ui::ReinforcementArgsDialog)
{
    m_ui->setupUi(getWindowContentWidget());
    m_height = MIN_HEIGHT;
    init();
}

ReinforcementArgsDialog::~ReinforcementArgsDialog()
{
    delete m_ui;
    if (m_currentArgHandle)
    {
        m_currentArgHandle->deleteLater();
        m_currentArgHandle = nullptr;
    }
}

QSize ReinforcementArgsDialog::sizeHint() const
{
    /*根据系统分辨率设置窗口大小*/
    QDesktopWidget *desktop = QApplication::desktop();
    KLOG_DEBUG("desktop width: %d,desktop height: %d", desktop->width(), desktop->height());
    QSize windowSize;
    if (desktop->height() >= 776 && desktop->width() >= 948)  // 能显示全
    {
        windowSize = QSize(500, m_height);
    }
    else
    {
        windowSize = QSize(desktop->width(), desktop->height());
    }

    return windowSize;
}

void ReinforcementArgsDialog::init()
{
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setIcon(QIcon(":/images/logo"));
    setTitle(tr("Set reinforment args"));
    setTitleBarHeight(36);
    setFixedSize(520, 269);
    setWindowModality(Qt::ApplicationModal);
    setResizeable(false);

    connect(m_ui->m_reset, SIGNAL(clicked(bool)), this, SLOT(argReset()));
    connect(m_ui->m_ok, SIGNAL(clicked(bool)), this, SLOT(setArgs()));
}

void ReinforcementArgsDialog::addLine(const QString &name,
                                      const QString &argName,
                                      const QString &label,
                                      const QString &valueLimits,
                                      const QString &inputExample,
                                      const QJsonValue &jsonValue,
                                      KS::Protocol::WidgetType::Value widgetType,
                                      const QString &note)
{
    m_height += LINE_HEIGHT;

    if (m_height > MAX_HEIGHT)
    {
        m_height = MAX_HEIGHT;
    }
    this->setMinimumHeight(m_height);

    auto item = new QListWidgetItem(m_ui->m_listWidget, 0);
    item->setSizeHint(QSize(400, 96));
    item->setFlags(Qt::NoItemFlags);

    ArgHandle *argHandle = new ArgHandle(m_ui->m_listWidget,
                                         name,
                                         argName,
                                         label,
                                         valueLimits,
                                         inputExample,
                                         jsonValue,
                                         widgetType,
                                         note);
    m_argHandle.append(argHandle);
    connect(argHandle, SIGNAL(valueChanged(QString, QString, QString, KS::Protocol::WidgetType::Value)), this, SIGNAL(valueChanged(QString, QString, QString, KS::Protocol::WidgetType::Value)));

    m_ui->m_listWidget->setItemWidget(item, argHandle);
}

void ReinforcementArgsDialog::clear()
{
    m_height = MIN_HEIGHT;
    m_ui->m_listWidget->clear();
}

void ReinforcementArgsDialog::closeEvent(QCloseEvent *e)
{
    m_argHandle.clear();
    e->accept();
    emit closed();
    QWidget::closeEvent(e);
}

void ReinforcementArgsDialog::setArgs()
{
    // 参数检测
    for (auto argHandle : m_argHandle)
    {
        CONTINUE_IF_TRUE(argHandle->valueCheck())

        if (argHandle->getWidgetType() == KS::Protocol::WidgetType::Value::DATETIME)
        {
            emit argError(QString(tr("%1 not less than %2")).arg(argHandle->getLabel()).arg(argHandle->getValueLimits()));
        }
        else if (argHandle->getWidgetType() == KS::Protocol::WidgetType::Value::TEXT)
        {
            emit argError(QString(tr("%1 input format is incorrect, please input the correct content according to the prompt")).arg(argHandle->getLabel()));
        }
        return;
    }

    m_argHandle.clear();
    emit okClicked();
    close();
}

int ReinforcementArgsDialog::getHeight()
{
    return m_height;
}

void ReinforcementArgsDialog::argReset()
{
    for (auto argHandle : m_argHandle)
    {
        m_currentArgHandle = argHandle;
        QString categoryName = argHandle->getCategoryName();
        QString argName = argHandle->getArgName();
        KLOG_DEBUG() << "argReset! " << categoryName.toStdString().c_str() << argName.toStdString().c_str();
        emit reseted(categoryName, argName);
    }
}

void ReinforcementArgsDialog::setValue(const QJsonValue &jsonValue)
{
    m_currentArgHandle->setValue(jsonValue);
}
}  // namespace BR
}  // namespace KS
