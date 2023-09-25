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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "src/ui/common/pagination.h"
#include <QIntValidator>
#include <QPushButton>
#include "sc-marcos.h"
#include "src/ui/ui_pagination.h"

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
    m_ui->m_input->setValidator(new QIntValidator(m_ui->m_input));

    initPageButton();
    hide();

    connect(m_ui->m_input, &QLineEdit::returnPressed, this, &Pagination::jump);
    connect(m_ui->m_jumper, &QPushButton::clicked, this, &Pagination::jump);
    connect(m_ui->m_prev, &QPushButton::clicked, this, &Pagination::prevClick);
    connect(m_ui->m_next, &QPushButton::clicked, this, &Pagination::nextClick);
    //更新页码按钮
    connect(this, &Pagination::totalPageChanged, this, &Pagination::updatetPageBtns);
    //更新页码按钮
    connect(this, &Pagination::currentPageChanged, this, &Pagination::setSelectedBtn);
}

Pagination::~Pagination()
{
    delete m_ui;
}

void Pagination::setTotalPages(int number)
{
    m_totalPage = number;
    emit totalPageChanged();
}

void Pagination::initPageButton()
{
    m_pageButtons = new QList<QPushButton *>();
    for (int i = 0; i < PAGE_BUTTON_SIZE; ++i)
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
    auto number = m_ui->m_input->text();
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

void Pagination::updatetPageBtns()
{
    //只有一页（8条）时隐藏分页控件
    if (m_totalPage <= 1)
        this->hide();
    else
    {
    }
}

void Pagination::setSelectedBtn()
{
    //如果被点击的btn显示在界面中（已有选中状态），则不处理
}

}  // namespace KS
