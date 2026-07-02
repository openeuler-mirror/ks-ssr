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
#ifndef TRUSTEDVIEW_H
#define TRUSTEDVIEW_H

#include <QWidget>
#include "src/ui/trusted-protected/table-common.h"
#include "src/ui/trusted-protected/trusted-table.h"

namespace KS
{
class TrustedView : public TableCommon
{
    Q_OBJECT
public:
    TrustedView(QWidget *parent = nullptr, TRUSTED_PROTECT_TYPE type = KERNEL_PROTECT);
    virtual ~TrustedView(){};

private:
    void initUI();
    void initBtns();
    // 是否需要初始化数据
    void isNeedInitData();

private slots:
    void addClicked(bool checked);
    void updateClicked(bool checked);
    void delClicked(bool checked);
    void searchTextChanged(const QString &text);
    void initFinished();

private:
    TPTable *m_trustedTable;
    QList<QPushButton *> m_operateBtnList;

    TrustedProxy *m_trustedProxy;

    TRUSTED_PROTECT_TYPE m_type;
};
}  // namespace KS

#endif  // TRUSTEDVIEW_H
