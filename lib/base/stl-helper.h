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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

// #include <map>
// #include <vector>
#include <QMap>
#include <QVector>

namespace KS
{
class MapHelper
{
public:
    MapHelper(){};
    virtual ~MapHelper(){};

    template <typename K, typename V>
    static QVector<V> getKeys(const QMap<K, V> &maps)
    {
        QVector<K> values;
        for (auto iter = maps.begin(); iter != maps.end(); ++iter)
        {
            values.push_back(iter->first);
        }
        return values;
    }

    template <typename K, typename V>
    static QVector<V> getValues(const QMap<K, V> &maps)
    {
        QVector<V> values;
        for (auto iter = maps.begin(); iter != maps.end(); ++iter)
        {
            values.push_back(iter.value());
        }
        return values;
    }

    template <typename K, typename V>
    static V getValue(const QMap<K, V> &maps, const K &key)
    {
        auto iter = maps.find(key);
        if (iter != maps.end())
        {
            return iter.value();
        }
        return V();
    }
};

}  // namespace KS