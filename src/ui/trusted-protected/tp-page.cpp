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

#include "src/ui/trusted-protected/tp-page.h"
#include <QHBoxLayout>

namespace KS
{
TPPage::TPPage(QWidget *parent) : QWidget(parent),
                                  m_listWidget(nullptr),
                                  m_itemKernelProxy(nullptr),
                                  m_itemExecuteProxy(nullptr),
                                  m_stackWidget(nullptr)
{
    this->initUI();
}

void TPPage::initUI()
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *list = new QWidget(this);
    m_listWidget = new QListWidget(list);
    createItem(tr("Execute protecked"), TRUSTED_PROTECT_TYPE::EXECUTE_PROTECT, ":/images/execution-control");
    createItem(tr("Kernel protecked"), TRUSTED_PROTECT_TYPE::KERNEL_PROTECT, ":/images/kernel-module-protected");
    m_listWidget->setCurrentRow(0);
    m_listWidget->setSpacing(8);
    m_listWidget->setFixedWidth(184);

    list->setFixedSize(200, 476);
    auto *listlay = new QHBoxLayout(list);
    listlay->setContentsMargins(0, 0, 0, 0);
    listlay->addWidget(m_listWidget);
    list->setContentsMargins(2, 8, 16, 0);

    connect(m_listWidget, &QListWidget::itemClicked, this, &TPPage::onItemClicked);

    m_executeView = new TrustedView(this, TRUSTED_PROTECT_TYPE::EXECUTE_PROTECT);
    m_kernelView = new TrustedView(this, TRUSTED_PROTECT_TYPE::KERNEL_PROTECT);

    m_stackWidget = new QStackedWidget(this);
    m_stackWidget->addWidget(m_executeView);
    m_stackWidget->addWidget(m_kernelView);
    m_stackWidget->setCurrentIndex(0);

    connect(this, SIGNAL(currentItemChanged(int)), m_stackWidget, SLOT(setCurrentIndex(int)));

    auto *space = new QWidget(this);
    space->setFixedWidth(4);
    space->setStyleSheet("background-color: #222222;");

    mainLayout->addWidget(list);
    mainLayout->addWidget(space);
    mainLayout->addWidget(m_stackWidget);
}

void TPPage::createItem(const QString &text,
                        TRUSTED_PROTECT_TYPE type,
                        const QString &icon)
{
    auto *item = new QListWidgetItem(m_listWidget);

    if (type == TRUSTED_PROTECT_TYPE::EXECUTE_PROTECT)
    {
        m_itemExecuteProxy = new ItemProxy(text, icon, type, m_listWidget);
        m_itemExecuteProxy->setRightShow(true);

        m_listWidget->addItem(item);
        m_listWidget->setItemWidget(item, m_itemExecuteProxy);
    }
    else
    {
        m_itemKernelProxy = new ItemProxy(text, icon, type, m_listWidget);

        m_listWidget->addItem(item);
        m_listWidget->setItemWidget(item, m_itemKernelProxy);
    }
    item->setTextAlignment(Qt::AlignVCenter);

    item->setData(Qt::UserRole, type);
    item->setSizeHint(QSize(166, 50));
}

void TPPage::onItemClicked(QListWidgetItem *currItem)
{
    auto currData = currItem->data(Qt::UserRole).toInt();

    if (currData == TRUSTED_PROTECT_TYPE::EXECUTE_PROTECT)
    {
        m_itemExecuteProxy->setRightShow(true);
        m_itemKernelProxy->setRightShow(false);
    }
    else
    {
        m_itemExecuteProxy->setRightShow(false);
        m_itemKernelProxy->setRightShow(true);
    }

    emit currentItemChanged(currData);
}

}  // namespace KS
