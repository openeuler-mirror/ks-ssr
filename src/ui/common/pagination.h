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

#pragma once

#include <QPushButton>
#include <QWidget>

namespace Ui
{
class Pagination;
}

namespace KS
{
class PageButtonGroup;
class Pagination : public QWidget
{
    Q_OBJECT

public:
    Pagination(QWidget *parent = nullptr);
    virtual ~Pagination();
    void setTotalPages(int number);

private:
    void initPageButton();
    void setCurrentPage(int currentPage);

private slots:
    void jump();
    void prevClick();
    void nextClick();
    void pageBtnClick();
    void updatePageBtns(int number);

signals:
    void jumperClicked(int number);
    void currentPageChanged(int number);
    void totalPageChanged(int number);

private:
    Ui::Pagination *m_ui;
    int m_totalPage;
    int m_prevPage;
    int m_currentPage;
    QList<QPushButton *> *m_pageButtons;
};
}  // namespace KS
