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

#pragma once

#include <QObject>

namespace KS
{
class ThreadPool;

namespace BR
{
class Configuration;
class Categories;
class Plugins;
class BRDBus;
class JobManager;

class Context : public QObject
{
    Q_OBJECT

public:
    static void globalInit();
    static void globalDeinit()
    {
        delete m_instance;
    };

    static Context *getInstance()
    {
        return m_instance;
    }

    Plugins *getPlugins()
    {
        return m_plugins;
    }

    ThreadPool *getThreadPool()
    {
        return m_threadPool;
    }

private:
    Context();
    virtual ~Context();
    void init();

private:
    static Context *m_instance;
    Configuration *m_configuration;
    Categories *m_categories;
    Plugins *m_plugins;
    BRDBus *m_dbus;
    JobManager *m_jobManager;
    // 线程池
    ThreadPool *m_threadPool;
};
}  // namespace BR
}  // namespace KS