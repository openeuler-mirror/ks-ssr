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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "table-filter-model.h"
#include "include/ksc-marcos.h"

namespace KS
{
TableFilterModel::TableFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool TableFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QString textComb;
    for (auto i = 0; i < sourceModel()->columnCount(); ++i)
    {
        auto index = this->sourceModel()->index(sourceRow, i, sourceParent);
        auto text = this->sourceModel()->data(index).toString();
        RETURN_VAL_IF_TRUE(text.contains(this->filterRegExp()), true);
    }

    return false;
}
}  // namespace KS
