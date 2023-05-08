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
#ifndef TPKERNELDELEGATE_H
#define TPKERNELDELEGATE_H

#include <QObject>
#include "src/ui/tp/tp-delegate.h"

namespace KS
{
// 防卸载列数
#define PROHIBIT_UNLOADING_COL 4

class TPKernelDelegate : public TPDelegate
{
    Q_OBJECT
public:
    TPKernelDelegate(QObject *parent = nullptr);
    virtual ~TPKernelDelegate(){};

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
};
}  // namespace KS
#endif  // TPKERNELDELEGATE_H
