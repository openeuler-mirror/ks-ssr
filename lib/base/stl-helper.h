/**
 * @file          /ks-ssr-manager/lib/base/stl-helper.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include <map>
#include <vector>

namespace KS
{
class MapHelper
{
public:
    MapHelper(){};
    virtual ~MapHelper(){};

    template <typename K, typename V>
    static std::vector<V> get_keys(const std::map<K, V> &maps)
    {
        std::vector<K> values;
        for (auto iter = maps.begin(); iter != maps.end(); ++iter)
        {
            values.push_back(iter->first);
        }
        return values;
    }

    template <typename K, typename V>
    static std::vector<V> get_values(const std::map<K, V> &maps)
    {
        std::vector<V> values;
        for (auto iter = maps.begin(); iter != maps.end(); ++iter)
        {
            values.push_back(iter->second);
        }
        return values;
    }

    template <typename K, typename V>
    static V get_value(const std::map<K, V> &maps, const K &key)
    {
        auto iter = maps.find(key);
        if (iter != maps.end())
        {
            return iter->second;
        }
        return V();
    }
};

}  // namespace KS