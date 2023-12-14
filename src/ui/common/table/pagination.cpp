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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "src/ui/common/table/pagination.h"
#include <QIntValidator>
#include <QPushButton>
#include "src/ui/ui_pagination.h"
#include "ssr-marcos.h"

//页码按钮个数
#define PAGE_BUTTON_SIZE 6

//分割按钮所在的位置编号，从0开始
#define SEPARATE_BUTTON_NUM 4
namespace KS
{
Pagination::Pagination(QWidget *parent) : QWidget(parent),
                                          m_ui(new Ui::Pagination)
{
    m_ui->setupUi(this);
    m_ui->m_pageNoInput->setValidator(new QIntValidator(m_ui->m_pageNoInput));

    initPageButton();
    hide();

    connect(m_ui->m_pageNoInput, &QLineEdit::returnPressed, this, &Pagination::jump);
    connect(m_ui->m_pageJumper, &QPushButton::clicked, this, &Pagination::jump);

    //m_ui->m_prev: 跳转至上一页的按钮
    //m_ui->m_next: 跳转至下一页的按钮
    connect(m_ui->m_prev, &QPushButton::clicked, this, &Pagination::prevClick);
    connect(m_ui->m_next, &QPushButton::clicked, this, &Pagination::nextClick);
    //更新页码按钮
    connect(this, &Pagination::totalPageChanged, this, &Pagination::updatePageBtns);
    //更新页码按钮
    connect(this, &Pagination::currentPageChanged, this, &Pagination::updatePageBtns);
}

Pagination::~Pagination()
{
    delete m_ui;
}

void Pagination::setTotalPages(int number)
{
    m_totalPage = number;
    emit totalPageChanged(m_totalPage);
}

void Pagination::initPageButton()
{
    m_pageButtons = new QList<QPushButton *>();
    for (auto i = 0; i < PAGE_BUTTON_SIZE; ++i)
    {
        auto *btn = new QPushButton(QString::number(i + 1), this);

        m_pageButtons->append(btn);

        if (i < SEPARATE_BUTTON_NUM)
        {
            m_ui->m_leftLayout->addWidget(btn);
        }
    }
}

void Pagination::setCurrentPage(int currentPage)
{
    m_prevPage = m_currentPage;
    m_currentPage = currentPage;
    emit currentPageChanged(m_currentPage);
}

void Pagination::jump()
{
    auto number = m_ui->m_pageNoInput->text();
    RETURN_IF_TRUE(number.isEmpty() ||
                   0 > number.toInt() ||
                   number.toInt() > m_totalPage);

    setCurrentPage(number.toInt());
}

void Pagination::prevClick()
{
    RETURN_IF_TRUE(m_currentPage == 1);
    setCurrentPage(m_currentPage - 1);
}

void Pagination::nextClick()
{
    RETURN_IF_TRUE(m_currentPage == m_totalPage);
    setCurrentPage(m_currentPage + 1);
}

void Pagination::pageBtnClick()
{
    auto btn = qobject_cast<QPushButton *>(sender());
    m_currentPage = btn->text().toInt();
    setCurrentPage(m_currentPage);
}

void Pagination::updatePageBtns(int number)
{
    //只有一页（8条）时隐藏分页控件
    if (m_totalPage <= 1)
        hide();
}

}  // namespace KS
