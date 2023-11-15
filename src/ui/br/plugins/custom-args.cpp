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

#include "custom-args.h"
#include <qt5-log-i.h>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QHeaderView>
#include "include/ssr-marcos.h"
#include "ui_custom-args.h"

namespace KS
{
namespace BR
{
namespace Plugins
{
CustomArgs::CustomArgs(QWidget *parent) : TitlebarWindow(parent),
                                          m_currentArgHandle(nullptr),
                                          m_ui(new Ui::CustomArgs)
{
    m_ui->setupUi(getWindowContentWidget());
    m_height = MIN_HEIGHT;
    init();
}

CustomArgs::~CustomArgs()
{
    delete m_ui;
    if (m_currentArgHandle)
    {
        m_currentArgHandle->deleteLater();
        m_currentArgHandle = nullptr;
    }
}

QSize CustomArgs::sizeHint() const
{
    /*根据系统分辨率设置窗口大小*/
    QDesktopWidget *desktop = QApplication::desktop();
    KLOG_DEBUG("desktop width: %d,desktop height: %d", desktop->width(), desktop->height());
    QSize windowSize;
    if (desktop->height() >= 776 && desktop->width() >= 948)  //能显示全
    {
        windowSize = QSize(500, m_height);
    }
    else
    {
        windowSize = QSize(desktop->width(), desktop->height());
    }

    return windowSize;
}

void CustomArgs::init()
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

void CustomArgs::addOneLine(const QString &name,
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
    //connect(argHandle, SIGNAL(reset(QString,QString)), this, SLOT(argReset(QString,QString)));
    connect(argHandle, SIGNAL(valueChanged(QString, QString, QString, KS::Protocol::WidgetType::Value)), this, SLOT(argChanged(QString, QString, QString, KS::Protocol::WidgetType::Value)));

    m_ui->m_listWidget->setItemWidget(item, argHandle);
    //    m_ui->m_listWidget->show();
}

void CustomArgs::clear()
{
    m_height = MIN_HEIGHT;
    m_ui->m_listWidget->clear();
}

void CustomArgs::closeEvent(QCloseEvent *e)
{
    m_argHandle.clear();
    e->accept();
    emit closed();
    QWidget::closeEvent(e);
}

void CustomArgs::argChanged(const QString &name,
                            const QString &argName,
                            const QString &value,
                            KS::Protocol::WidgetType::Value widgetType)
{
    emit valueChanged(name, argName, value, widgetType);
}

void CustomArgs::setArgs()
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

int CustomArgs::getHeight()
{
    return m_height;
}

void CustomArgs::argReset()
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

void CustomArgs::setValue(const QJsonValue &jsonValue)
{
    m_currentArgHandle->setValue(jsonValue);
}
}  // namespace Plugins
}  // namespace BR
}  // namespace KS
