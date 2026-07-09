/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#include "context.h"
#include "categories.h"
#include "configuration.h"
#include "dbus.h"
#include "job-dispatcher.h"
#include "lib/base/thread-pool.h"
#include "plugins.h"

namespace KS
{
namespace BR
{
Context* Context::m_instance = NULL;
void Context::globalInit()
{
    m_instance = new Context();
    m_instance->init();
}

Context::Context()
    : m_threadPool(nullptr)
{
    m_configuration = new Configuration(SSR_INSTALL_DATADIR "/ssr.ini", this);
    m_categories = new Categories(this);
    m_plugins = new Plugins(m_configuration, this);
    m_jobManager = new JobManager(m_configuration, m_plugins, this);
    m_dbus = new BRDBus(m_configuration, m_categories, m_plugins, m_jobManager, this);
}

Context::~Context()
{
    if (m_threadPool)
    {
        delete m_threadPool;
        m_threadPool = nullptr;
    }
}

void Context::init()
{
    m_configuration->init();
    m_categories->init();
    m_plugins->init();
    m_dbus->init();
    m_jobManager->init();

    m_threadPool = new ThreadPool(m_configuration->getMaxThreadNum());
}

}  // namespace BR
}  // namespace KS
