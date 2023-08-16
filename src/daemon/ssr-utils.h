/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include <glib.h>
#include <glib/gi18n.h>
#include <xsd/cxx/tree/containers.hxx>

namespace Kiran
{
class SSRUtils
{
public:
    template <typename T>
    static std::string get_xsd_local_value(const ::xsd::cxx::tree::sequence<T> &values)
    {
        auto language_names = g_get_language_names();
        // 查找对应语言的选项
        for (int32_t i = 0; language_names[i]; ++i)
        {
            for (const auto &value : values)
            {
                if (value.lang().present() && value.lang().get() == language_names[i])
                {
                    return std::string(value);
                }
            }
        }
        // 如果未找到，则使用默认的英文
        for (const auto &value : values)
        {
            if (!value.lang().present())
            {
                return std::string(value);
            }
        }
        return std::string();
    }
};

}  // namespace Kiran
