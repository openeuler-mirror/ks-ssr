/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * kiran-session-manager is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/ui/window.h"
#include <QPushButton>
#include <QStackedWidget>
#include "src/ui/box/box-manager.h"
#include "src/ui/navigation.h"
#include "src/ui/ui_window.h"

namespace KS
{
Window::Window() : QWidget(nullptr),
                   m_ui(new Ui::Window())
{
    m_ui->setupUi(this);

    // 初始化分类选项
    this->m_ui->m_navigation->addItem(new NavigationItem(":/box-little"));

    // 移除qt designer默认创建的widget
    while (this->m_ui->m_categoryPages->currentWidget() != nullptr)
    {
        auto currentWidget = this->m_ui->m_categoryPages->currentWidget();
        this->m_ui->m_categoryPages->removeWidget(currentWidget);
        delete currentWidget;
    }

    this->m_ui->m_categoryPages->addWidget(new BoxManager());
    this->m_ui->m_categoryPages->setCurrentIndex(0);

    connect(this->m_ui->m_navigation, SIGNAL(currentCategoryChanged(int)), this->m_ui->m_categoryPages, SLOT(setCurrentIndex(int)));
}
}  // namespace KS
