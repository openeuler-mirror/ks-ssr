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

#include <QWidget>

namespace Ui
{
class Pagination;
}

class QPushButton;
class QLineEdit;
class QLabel;
class QIntValidator;

namespace KS
{
class Pagination : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Pagination:初始化页面选择界面，使用该函数初始化为多按钮选择
     * @param[in] totalPage   需要创建的页面选择界面控制的最大页数
     * @param[in] maxShowPages 必须大于等于5，建议最大不不超过8
     * @param[in] jumpEdit 是否需要页面跳转
     */
    explicit Pagination(int totalPage, int maxShowPages, bool jumpEdit, QWidget *parent = NULL);
    /**
     * @brief Pagination:初始化页面选择界面，使用该函数初始化为数字显示
     * @param[in] totalPage   需要创建的页面选择界面控制的最大页数
     */
    explicit Pagination(int totalPage, QWidget *parent = NULL);
    ~Pagination();

    int getCurrentPage();
    void gotoFirstPage();
    void setTotalPage(int totalPage);

    void forceChangeCurrentPage(int currentPage);
signals:
    void currentPageChanged(int page);

private:
    void initUI();
    /**
     * @param[in] totalPage   需要创建的页面选择界面控制的最大页数
     * @param[in] currentPage 必须大于等于1，不得超过totalPage
     */
    void setTotalPage(int totalPage, int currentPage);
    void setCheckedButton(int currentPage);
    void updateTotalPageLable(int totalPage);
    void setWidth(int width);
    void handleFirstLastPage(int currentPage);
    void emptyPageBtnList();
    void createPageBtnList(int btnListSize);

private slots:
    void jumpPage();
    void clickedSelectButton();
    void clickedLeftPushButton();
    void clickedRightPushButton();

private:
    enum PageSelectMode
    {
        PageSelectByButton,
        PageSelectByNo
    };

    Ui::Pagination *m_ui;
    int m_totalPage, m_maxShowPages, m_currentPage;
    QList<QPushButton *> m_buttonList;
    QLineEdit *m_jumpLineEdit;
    QLabel *m_totalPageLable;
    int m_pageSelectMode;
    QIntValidator *m_intValidator;
};
}  // namespace KS
