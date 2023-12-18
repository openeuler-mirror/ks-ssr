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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "table-filter-model.h"
#include <qt5-log-i.h>
#include "device-list-table.h"
#include "include/ssr-marcos.h"
#include "utils.h"

namespace KS
{
namespace DM
{
TableFilterModel::TableFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

void TableFilterModel::setSearchText(const QString &text)
{
    m_searchText = text;
}

bool TableFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // 适用于有多个表头筛选列的情况下，正则由格式为 (表头1正则).*(表头n正则)，若没有).*(，则代表有一列是没有选中的筛选项的，表格不需要显示数据
    RETURN_VAL_IF_TRUE(filterRegExp().isEmpty() || !filterRegExp().pattern().contains(").*("), false)
    QString sourceString;
    // 拼接一行的数据
    for (auto i = 0; i < LIST_TABLE_FIELD_LAST; ++i)
    {
        auto index = sourceModel()->index(sourceRow, i, sourceParent);
        auto text = sourceModel()->data(index).toString();
        // 状态栏为枚举值，需要进行转换
        if (i == LIST_TABLE_FIELD_STATUS)
        {
            text = Utils::deviceStateEnum2Str(DeviceState(sourceModel()->data(index).toInt()));
        }
        sourceString += text;
    }

    if (!m_searchText.isEmpty())
    {
        RETURN_VAL_IF_TRUE(sourceString.contains(m_searchText) && sourceString.contains(filterRegExp()), true);
    }
    else
    {
        RETURN_VAL_IF_TRUE(sourceString.contains(filterRegExp()), true);
    }

    return false;
}
}  // namespace DM
}  // namespace KS
