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
#include "pagination.h"
#include "ui_pagination.h"

#include <kiran-log/qt5-log-i.h>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include "include/ssr-marcos.h"

namespace KS
{
Pagination::Pagination(int totalPage, int maxShowPages, bool jumpEdit, QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::Pagination)
{
    m_ui->setupUi(this);
    m_totalPage = totalPage;
    m_maxShowPages = maxShowPages;
    m_currentPage = 1;
    m_pageSelectMode = PageSelectByButton;
    if (jumpEdit)
    {
        m_jumpLineEdit = new QLineEdit(this);
        m_jumpLineEdit->setFixedSize(QSize(72, 30));
        m_ui->horizontalLayout_jump->addWidget(m_jumpLineEdit);
        m_jumpLineEdit->setValidator(new QIntValidator(this));
        m_jumpLineEdit->setPlaceholderText(tr("Input page"));
        connect(m_jumpLineEdit, SIGNAL(returnPressed()), this, SLOT(jumpPage()));
    }
    initUI();
    connect(m_ui->m_goto, &QPushButton::clicked, this, &Pagination::jumpPage);
    setTotalPage(totalPage, m_currentPage);
    m_totalPage > 1 ? show() : hide();
}

Pagination::Pagination(int totalPage, QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::Pagination)
{
    m_ui->setupUi(this);
    m_totalPage = totalPage;
    m_ui->m_buttonLast->hide();
    m_currentPage = 1;
    m_pageSelectMode = PageSelectByNo;
    m_ui->m_left->setMaximumSize(80, 30);
    m_ui->m_left->setMinimumSize(80, 30);
    m_ui->m_left->setText(QObject::tr("PREV"));

    m_ui->m_right->setMinimumSize(80, 30);
    m_ui->m_right->setMaximumSize(80, 30);
    m_ui->m_right->setText(QObject::tr("NEXT"));

    m_ui->m_buttonLayout->addItem(new QSpacerItem(20, 30));

    m_jumpLineEdit = new QLineEdit(this);
    m_jumpLineEdit->setMinimumSize(QSize(50, 30));
    m_jumpLineEdit->setAlignment(Qt::AlignHCenter);
    m_ui->m_buttonLayout->addWidget(m_jumpLineEdit);
    m_jumpLineEdit->setValidator(new QIntValidator(this));
    m_jumpLineEdit->setPlaceholderText("1");

    auto labelLine = new QLabel("-", this);
    labelLine->setMaximumSize(QSize(22, 30));
    labelLine->setMinimumSize(QSize(22, 30));
    labelLine->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_ui->m_buttonLayout->addWidget(labelLine);

    m_totalPageLable = new QLabel(this);
    m_totalPageLable->setMaximumSize(QSize(50, 30));
    m_totalPageLable->setMinimumSize(QSize(50, 30));
    m_totalPageLable->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_ui->m_buttonLayout->addWidget(m_totalPageLable);

    m_ui->m_buttonLayout->addItem(new QSpacerItem(20, 30));

    updateTotalPageLable(totalPage);

    connect(m_jumpLineEdit, SIGNAL(returnPressed()), this, SLOT(jumpPage()));

    initUI();
    setTotalPage(totalPage, m_currentPage);
    setWidth(336);
}

Pagination::~Pagination()
{
    delete m_ui;
}

void Pagination::initUI()
{
    connect(m_ui->m_left, &QPushButton::clicked, this, &Pagination::clickedLeftPushButton);
    connect(m_ui->m_right, &QPushButton::clicked, this, &Pagination::clickedRightPushButton);

    RETURN_IF_TRUE(m_pageSelectMode != PageSelectByButton);
    m_ui->m_buttonLast->setCheckable(true);
    m_ui->m_buttonLast->setAutoExclusive(true);
    m_ui->m_buttonLast->setText(QString("%1").arg(m_totalPage));
    connect(m_ui->m_buttonLast, SIGNAL(clicked()), this, SLOT(clickedSelectButton()));
}

void Pagination::jumpPage()
{
    setTotalPage(m_totalPage, m_jumpLineEdit->text().toInt());
    emit currentPageChanged(m_currentPage);
}

void Pagination::clickedSelectButton()
{
    auto btn = qobject_cast<QPushButton *>(sender());
    RETURN_IF_TRUE(btn->text().toInt() == m_currentPage)

    setTotalPage(m_totalPage, btn->text().toInt());
    emit currentPageChanged(m_currentPage);
}

void Pagination::handleFirstLastPage(int currentPage)
{
    if (currentPage == 1 && currentPage != m_totalPage)
    {
        m_ui->m_left->setEnabled(false);
        m_ui->m_right->setEnabled(true);
    }
    else if (currentPage == m_totalPage && currentPage != 1)
    {
        m_ui->m_right->setEnabled(false);
        m_ui->m_left->setEnabled(true);
    }
    else if (currentPage == 1 && currentPage == m_totalPage)
    {
        m_ui->m_right->setEnabled(false);
        m_ui->m_left->setEnabled(false);
    }
    else
    {
        m_ui->m_left->setEnabled(true);
        m_ui->m_right->setEnabled(true);
    }
}

void Pagination::emptyPageBtnList()
{
    while (!m_buttonList.isEmpty())
    {
        delete m_buttonList.first();
        m_buttonList.removeFirst();
    }
}

void Pagination::createPageBtnList(int btnListSize)
{
    for (int i = 1; i < btnListSize; i++)
    {
        auto btn = new QPushButton(this);
        btn->setMaximumSize(QSize(30, 30));
        btn->setMinimumSize(QSize(30, 30));
        btn->setText(QString::number(i));
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        connect(btn, SIGNAL(clicked()), this, SLOT(clickedSelectButton()));
        m_ui->m_buttonLayout->addWidget(btn);
        m_buttonList.append(btn);
    }
}

void Pagination::setTotalPage(int totalPage, int currentPage)
{
    if (currentPage > totalPage || currentPage <= 0)
    {
        KLOG_WARNING() << QString("current page overrange:current page = %1").arg(QString::number(currentPage));
        return;
    }
    m_ui->m_buttonLast->setText(QString("%1").arg(totalPage));
    m_totalPage = totalPage;
    m_currentPage = currentPage;

    // 首尾页判断
    handleFirstLastPage(currentPage);

    if (m_pageSelectMode == PageSelectByNo)
    {
        updateTotalPageLable(totalPage);
        m_jumpLineEdit->setText(QString::number(currentPage));
        return;
    }

    if (totalPage <= m_maxShowPages)
    {
        // 清空链表重新插入
        emptyPageBtnList();
        createPageBtnList(totalPage);
        if (currentPage == totalPage)
        {
            m_ui->m_buttonLast->setChecked(true);
        }
    }
    else
    {
        // 创建按键
        if (m_buttonList.length() < m_maxShowPages - 1)
        {
            emptyPageBtnList();
            createPageBtnList(m_maxShowPages);
        }
        // 按键逻辑
        if (currentPage <= m_maxShowPages - 3)
        {
            m_buttonList.last()->setText("...");
            m_buttonList.last()->setEnabled(false);
            m_buttonList.first()->setEnabled(true);
            for (int i = 0; i < m_maxShowPages - 2; i++)
            {
                m_buttonList[i]->setText(QString::number(i + 1));
            }
        }
        else if (currentPage >= totalPage - 3)
        {
            int pageNo = totalPage - 1;
            m_buttonList.first()->setText(("..."));
            m_buttonList.first()->setEnabled(false);
            m_buttonList.last()->setEnabled(true);
            for (auto iter = --m_buttonList.end(); iter != m_buttonList.begin(); iter--)
            {
                (*iter)->setText(QString::number(pageNo));
                pageNo--;
            }
        }
        else
        {
            int pageNo = currentPage + 1;
            m_buttonList.first()->setText(("..."));
            m_buttonList.first()->setEnabled(false);
            m_buttonList.last()->setText(("..."));
            m_buttonList.last()->setEnabled(false);
            for (int i = m_maxShowPages - 3; i >= 1; i--)
            {
                m_buttonList[i]->setText(QString::number(pageNo));
                pageNo--;
            }
        }
    }
    setCheckedButton(currentPage);
}

int Pagination::getCurrentPage()
{
    return m_currentPage;
}

void Pagination::gotoFirstPage()
{
    setTotalPage(m_totalPage, 1);
}

void Pagination::forceChangeCurrentPage(int currentPage)
{
    RETURN_IF_TRUE(currentPage <= 0);
    m_currentPage = currentPage;
}

void Pagination::setTotalPage(int totalPage)
{
    setTotalPage(totalPage, m_currentPage);
    m_totalPage > 1 ? show() : hide();
}

void Pagination::setCheckedButton(int currentPage)
{
    if (currentPage == m_totalPage)
    {
        m_ui->m_buttonLast->setChecked(true);
    }
    for (int i = 0; i < m_buttonList.length(); i++)
    {
        if (m_buttonList[i]->text().toInt() == currentPage)
        {
            m_buttonList[i]->setChecked(true);
        }
    }
}

void Pagination::updateTotalPageLable(int totalPage)
{
    m_totalPageLable->setText(QString::number(totalPage));
}

void Pagination::setWidth(int width)
{
    setMinimumWidth(width);
    setMaximumWidth(width);
}

void Pagination::clickedLeftPushButton()
{
    setTotalPage(m_totalPage, --m_currentPage);
    emit currentPageChanged(m_currentPage);
}

void Pagination::clickedRightPushButton()
{
    setTotalPage(m_totalPage, ++m_currentPage);
    emit currentPageChanged(m_currentPage);
}
}  // namespace KS
