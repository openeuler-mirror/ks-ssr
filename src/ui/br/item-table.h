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
#include <QCheckBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QWidget>
#include "br-i.h"
#include "include/ssr-i.h"
#include "src/ui/br/plugins/categories.h"
#include "src/ui/common/table-header-proxy.h"

namespace KS
{
namespace BR
{
struct ProgressInfo;

class ItemTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ItemTableDelegate(QObject *parent = nullptr);
    ~ItemTableDelegate();

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

class ItemTable : public QTreeView
{
    Q_OBJECT

public:
    explicit ItemTable(QWidget *parent = 0);
    ~ItemTable();

    virtual QSize sizeHint() const override;

    int getCount();
    void setItem(const QList<Plugins::Categories *> &list);
    void updateStatus(const QList<Plugins::Categories *> &list);
    void clearCheckedStatus(const QList<Plugins::Categories *> &list, BRReinforcementState state);
    void hideCheckBox(bool isHide);
    void setAllCheckBoxEditStatus(bool isCheckBoxEdit);
    QStringList getString(const QList<Plugins::Categories *> &list);
    QStringList getAllString(const QList<Plugins::Categories *> &list);
    void getProgressCount(const QList<Plugins::Categories *> &list, ProgressInfo &progressInfo);
    // 设置单个项选中
    void setArgChecked(const QString &matchLabel, bool checkStatus);
    // 检测单个项是否选中
    bool checkedArgStatus(const QString &matchLabel);
    // 检测所有项是否选中，传出选中项的label名
    QList<QString> checkedAllStatus();
    // 设置全选
    void setAllChecked(Qt::CheckState isChecked = Qt::Checked);

private:
    void initHeader();
    void appendRow(const QList<Plugins::Category *> &list);
    void setIcon(const QList<Plugins::Categories *> &list, int i);
    void setItemArrow(const QModelIndex &model);

protected:
    void checkChanged(QStandardItem *item);
    void checkAllChild(QStandardItem *item, bool check);
    void checkAllChildRecursion(QStandardItem *item, bool check);
    void checkChildChanged(QStandardItem *item);
    Qt::CheckState checkSibling(QStandardItem *item);

signals:
    void itemChanged(QStandardItem *);
    void modelEntered(const QModelIndex &model);
    void modelClicked(const QModelIndex &model);
    void modifyItemArgsClicked(const QModelIndex &model);

private slots:
    void selectAllItem(Qt::CheckState state);
    void setHeaderState(QStandardItem *x);
    void showTail(const QModelIndex &model);
    void setExpandItem(const QModelIndex &model);
    void doubleClickItem(const QModelIndex &model);

private:
    QStandardItemModel *m_model;
    int m_count = 0;

    TableHeaderProxy *m_headerProxy;
};

}  // namespace BR
}  // namespace KS
