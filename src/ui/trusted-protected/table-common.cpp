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

#include "src/ui/trusted-protected/table-common.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include "sc-i.h"
#include "src/ui/ui_table-common.h"
namespace KS
{
TableCommon::TableCommon(QWidget *parent) : QWidget(parent),
                                            m_ui(new Ui::TableCommon())
{
    this->m_ui->setupUi(this);

    // TODO:需要绘制颜色
    this->m_ui->m_search->addAction(QIcon(":/images/search"), QLineEdit::ActionPosition::LeadingPosition);

    connect(this->m_ui->m_search, &QLineEdit::textChanged, this, &TableCommon::sigSearchTextChanged);
}

void TableCommon::addOperationButton(QList<QPushButton *> btns)
{
    auto *hlay = new QHBoxLayout(m_ui->m_btns);
    hlay->setContentsMargins(0, 0, 0, 0);
    for (auto btn : btns)
    {
        hlay->addWidget(btn);
    }
}

void TableCommon::addTable(QTableView *table)
{
    m_ui->verticalLayout->addWidget(table);
}

void TableCommon::setPrompt(const QString &prompt)
{
    m_ui->m_prompt->setText(prompt);
}

void TableCommon::setSumTest(const QString &test)
{
    m_ui->m_tips->setText(test);
    m_ui->m_tips->setStyleSheet("QLabel{"
                                "color:#919191;"
                                "font:NotoSansCJKsc-Regular;"
                                "font-size:12px;}");
}

}  // namespace KS
