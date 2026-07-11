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
 * Author:     chendingjian <chendingjian@kylinsec.com.cn>
 */
#pragma once

#include <QCoreApplication>

namespace KS
{
namespace BR
{
class PluginsTranslation
{
public:
    static void globalInit()
    {
        m_instance = new PluginsTranslation();
    };
    static void globalDeinit()
    {
        if (m_instance)
        {
            delete m_instance;
        }
    };

    static PluginsTranslation *instance()
    {
        return m_instance;
    };

private:
    PluginsTranslation(){};
    ~PluginsTranslation(){};

private:
    static PluginsTranslation *m_instance;
};

PluginsTranslation *PluginsTranslation::m_instance = nullptr;
}  // namespace BR
}  // namespace KS
