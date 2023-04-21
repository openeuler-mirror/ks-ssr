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

#include <QListWidget>
#include <QStackedWidget>
#include <QStylePainter>
#include <QWidget>
#include "src/ui/trusted-protected/item-proxy.h"
#include "src/ui/trusted-protected/trusted-view.h"

namespace KS
{
class TPPage : public QWidget
{
    Q_OBJECT
public:
    TPPage(QWidget *parent = nullptr);
    virtual ~TPPage(){};

private:
    void initUI();
    void createItem(const QString &text,
                    TrustedProtectType type,
                    const QString &icon);

private slots:
    void onItemClicked(QListWidgetItem *currItem);

signals:
    void currentItemChanged(int index);

private:
    QListWidget *m_listWidget;
    ItemProxy *m_itemKernelProxy;
    ItemProxy *m_itemExecuteProxy;
    QStackedWidget *m_stackWidget;

    TrustedView *m_kernelView;
    TrustedView *m_executeView;
};

}  // namespace KS
