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

#pragma once

#include <QPushButton>

class QLabel;
class QHBoxLayout;
class QAction;
namespace KS
{
class HeaderButtonDelegate : public QPushButton
{
    Q_OBJECT
public:
    explicit HeaderButtonDelegate(QWidget *parent = nullptr);
    virtual ~HeaderButtonDelegate(){};

    void addMenuActions(QList<QAction *> actions);
    void setButtonText(const QString &text);
    QList<QAction *> getMenuActions() const;

private:
    void initUI();

signals:
    void menuTriggered(QAction *);

private:
    QLabel *m_text;
    QPushButton *m_icon;
    QHBoxLayout *m_layout;
    QMenu *m_menu;
};
}  // namespace KS
