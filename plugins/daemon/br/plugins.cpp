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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <QTimer>
#include "configuration.h"
#include "plugins.h"
#include "python/python-klog.h"
#include "utils.h"

namespace KS
{
namespace BR
{
#define PYTHON_CHECK_VERSION(major, minor, micro)                   \
    (PY_MAJOR_VERSION > (major) ||                                  \
     (PY_MAJOR_VERSION == (major) && PY_MINOR_VERSION > (minor)) || \
     (PY_MAJOR_VERSION == (major) && PY_MINOR_VERSION == (minor) && \
      PY_MICRO_VERSION >= (micro)))

Plugins::Plugins(Configuration* configuration, QObject* parent)
    : QObject(parent),
      m_configuration(configuration)
{
    m_loadReinforcementTimer = new QTimer(this);
    m_loadReinforcementTimer->setInterval(100);
}

Plugins::~Plugins()
{
    Py_Finalize();
}

void Plugins::init()
{
    // 内建模块的名称不支持package.module格式，因此这里不加br前缀了
    PyImport_AppendInittab("klog", PyInit_klog);

    constexpr const char* import_package_path = "sys.path.append('" SSR_BR_PLUGIN_PYTHON_ROOT_DIR "')";
    /* Python解析器不是线程安全的，Python解析器维护了一个全局锁(GIL)，多线程环境下，线程在执行Python的C API时需要先获取GIL，
       否则会导致数据异常。程序调用PyEval_InitThreads函数初始化时默认获取GIL，因此最开始是主线程拥有GIL，如果主线程未调用Python的C API，
       应该要释放掉GIL，否则其他线程在运行前无法获取到GIL，当主线程再次调用Python的C API时可以再去请求GIL。

       特别说明：当线程/主线程获取到GIL后，如果此时正在python脚本中执行IO/sleep等操作，Python解析器会负责对锁进行释放，等IO操作完成后重新请求GIL，
       相当于Python会自动执行如下代码：
       Py_BEGIN_ALLOW_THREADS // 释放锁
        ... Do some blocking I/O operation ...
       Py_END_ALLOW_THREADS   // 请求锁

       因此，虽然Python解析器同时只能有一个线程在运行，但不用担心线程获取到GIL后其他线程无法执行，Python解析器会根据实际情况进行优化，
       保证多个线程运行时可以进行切换。*/
#if !PYTHON_CHECK_VERSION(3, 9, 0)
    PyEval_InitThreads();
#endif
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(import_package_path);

    this->loadPlugins();
    this->loadReinforcements();

    // 这里对锁进行释放，确保其他线程可以获取到锁，如果主线程还需要操作Python解析器，则需要重新获取锁
    Utils::pyGiUnlock();

    connect(m_configuration, &Configuration::RSChanged, this, &Plugins::loadReinforcements);
    connect(m_configuration, &Configuration::StrategyChanged, this, &Plugins::loadReinforcements);
    connect(m_configuration, &Configuration::customRAChanged, this, &Plugins::loadReinforcements);
    // 前端设置加固参数后会立即获取新的，如果延时执行会导致前端界面显示不正确，因此这里先不用延时执行
    // connect(m_loadReinforcementTimer, &QTimer::timeout, this, &Plugins::loadReinforcements);
}

QSharedPointer<Plugin> Plugins::getPluginByReinforcement(const QString& name)
{
    auto iter = this->m_reinforcementsPlugins.find(name);
    RETURN_VAL_IF_TRUE(iter == this->m_reinforcementsPlugins.end(), QSharedPointer<Plugin>());
    return iter.value().lock();
}

BRReinforcementVec Plugins::getReinforcementsByCategory(const QString& category_name)
{
    BRReinforcementVec result;
    for (auto iter = this->m_reinforcements.begin(); iter != this->m_reinforcements.end(); ++iter)
    {
        if (iter.value()->getCategoryName() == category_name)
        {
            result.push_back(iter.value());
        }
    }
    return result;
}

QSharedPointer<BRReinforcementInterface> Plugins::getReinforcementInterface(const QString& plugin_name,
                                                                            const QString& reinforcement_name)
{
    auto plugin = this->getPlugin(plugin_name);
    if (!plugin)
    {
        KLOG_WARNING() << "Plugin '" << plugin_name.toLatin1() << "' of the reinforcement '" << reinforcement_name.toLatin1() << "' is not found.";
        return QSharedPointer<BRReinforcementInterface>();
    }

    auto plugin_interface = plugin->getLoader()->getInterface();
    if (!plugin_interface)
    {
        KLOG_WARNING() << "The Plugin interface for " << plugin_name.toLatin1() << " is NULL.";
        return QSharedPointer<BRReinforcementInterface>();
    }

    auto reinforcement_interface = plugin_interface->getReinforcement(reinforcement_name);
    if (!reinforcement_interface)
    {
        KLOG_WARNING() << "The reinforcement interface for " << reinforcement_name.toLatin1() << " is NULL.";
        return QSharedPointer<BRReinforcementInterface>();
    }
    return reinforcement_interface;
}

void Plugins::init()
{
    // 内建模块的名称不支持package.module格式，因此这里不加br前缀了
    PyImport_AppendInittab("klog", PyInit_klog);

    constexpr const char* import_package_path = "sys.path.append('" SSR_BR_PLUGIN_PYTHON_ROOT_DIR "')";
    /* Python解析器不是线程安全的，Python解析器维护了一个全局锁(GIL)，多线程环境下，线程在执行Python的C API时需要先获取GIL，
       否则会导致数据异常。程序调用PyEval_InitThreads函数初始化时默认获取GIL，因此最开始是主线程拥有GIL，如果主线程未调用Python的C API，
       应该要释放掉GIL，否则其他线程在运行前无法获取到GIL，当主线程再次调用Python的C API时可以再去请求GIL。

       特别说明：当线程/主线程获取到GIL后，如果此时正在python脚本中执行IO/sleep等操作，Python解析器会负责对锁进行释放，等IO操作完成后重新请求GIL，
       相当于Python会自动执行如下代码：
       Py_BEGIN_ALLOW_THREADS // 释放锁
        ... Do some blocking I/O operation ...
       Py_END_ALLOW_THREADS   // 请求锁

       因此，虽然Python解析器同时只能有一个线程在运行，但不用担心线程获取到GIL后其他线程无法执行，Python解析器会根据实际情况进行优化，
       保证多个线程运行时可以进行切换。*/
#if !PYTHON_CHECK_VERSION(3, 9, 0)
    PyEval_InitThreads();
#endif
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(import_package_path);

    this->loadPlugins();
    this->loadReinforcements();

    // 这里对锁进行释放，确保其他线程可以获取到锁，如果主线程还需要操作Python解析器，则需要重新获取锁
    Utils::pyGiUnlock();

    connect(m_configuration, &Configuration::RSChanged, this, /*&Plugins::idleLoadReinforcements*/ &Plugins::loadReinforcements);
    connect(m_configuration, &Configuration::StrategyChanged, this, /*&Plugins::idleLoadReinforcements*/ &Plugins::loadReinforcements);
    connect(m_configuration, &Configuration::customRAChanged, this, /*&Plugins::idleLoadReinforcements*/ &Plugins::loadReinforcements);
    connect(m_loadReinforcementTimer, &QTimer::timeout, this, &Plugins::loadReinforcements);
}

void Plugins::loadPlugins()
{
    KLOG_DEBUG("Plugins::loadPlugins");
    this->loadPluginsFromDir(SSR_BR_PLUGIN_CPP_ROOT_DIR);
    this->loadPluginsFromDir(SSR_BR_PLUGIN_PYTHON_ROOT_DIR);
    this->loadPluginsFromDir(SSR_BR_PLUGIN_BASH_ROOT_DIR);
}

void Plugins::loadPluginsFromDir(const QString& dirname)
{
    QDir pluginDir(dirname);

    for (auto iter : pluginDir.entryList(QDir::NoDotAndDotDot |
                                         QDir::Dirs |
                                         QDir::AllDirs |
                                         QDir::Files |
                                         QDir::Hidden))
    {
        auto basename = iter;
        auto filename = QDir::cleanPath(dirname + '/' + basename);
        if (!(basename.startsWith("br-plugin") && QFileInfo(filename).isFile()))
        {
            KLOG_DEBUG() << "Skip file " << filename.toLocal8Bit();
            continue;
        }
        auto plugin = QSharedPointer<Plugin>(new Plugin(filename));
        if (!plugin->init())
        {
            return;
        }
        auto pluginLoader = plugin->getLoader();
        if (!(pluginLoader->activate() && this->addPlugin(plugin)))
        {
            pluginLoader->deactivate();
        }
    }
}

bool Plugins::addPlugin(QSharedPointer<Plugin> plugin)
{
    RETURN_VAL_IF_FALSE(plugin, false);

    auto pluginId = plugin->getId();
    KLOG_DEBUG() << "plugin id: " << pluginId.toLatin1();

    if (this->m_plugins.find(pluginId) != this->m_plugins.end())
    {
        KLOG_WARNING() << "The plugin is already exist. id: %s." << pluginId;
        return false;
    }
    else
    {
        this->m_plugins[pluginId] = plugin;
    }

    auto reinforcement_names = plugin->getReinforcementNames();
    for (auto iter = reinforcement_names.begin(); iter != reinforcement_names.end(); ++iter)
    {
        auto& reinforcement_name = (*iter);
        auto old_plugin = this->getPluginByReinforcement(QString::fromStdString(reinforcement_name));
        if (old_plugin)
        {
            KLOG_WARNING() << "The reinforcement "
                           << reinforcement_name.c_str()
                           << " is conflicted with other plugin. old plugin: "
                           << old_plugin->getId().toLocal8Bit()
                           << ", cur plugin: "
                           << pluginId.toLocal8Bit();
        }
        else
        {
            this->m_reinforcementsPlugins[QString::fromStdString(reinforcement_name)] = plugin;
        }
    }
    return true;
}

void Plugins::idleLoadReinforcements()
{
    if (!m_loadReinforcementTimer->isActive())
    {
        m_loadReinforcementTimer->start();
    }
}

void Plugins::loadReinforcements()
{
    KLOG_DEBUG("Plugins::loadReinforcements");

    m_loadReinforcementTimer->stop();

    this->m_reinforcements.clear();

    auto rs = this->m_configuration->getRS();
    RETURN_IF_FALSE(rs);

    // 用自定义加固参数覆盖默认加固参数
    auto ra = this->m_configuration->getCustomRA();
    if (ra)
    {
        auto& custom_reinforcements = ra->reinforcement();
        for (auto custom_iter = custom_reinforcements.begin(); custom_iter != custom_reinforcements.end(); ++custom_iter)
        {
            auto& fixed_reinforcements = rs->body().reinforcement();
            for (auto fixed_iter = fixed_reinforcements.begin(); fixed_iter != fixed_reinforcements.end(); ++fixed_iter)
            {
                CONTINUE_IF_TRUE(custom_iter->name() != fixed_iter->name());
                this->joinReinforcement((*fixed_iter), (*custom_iter));
            }
        }
    }

    auto& reinforcements = rs->body().reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        auto& reinforcementArg = (*iter);
        auto reinforcementName = reinforcementArg.name();
        auto plugin = this->getPluginByReinforcement(QString::fromStdString(reinforcementName));

        // 加固标准中的加固项如果没有插件支持，则不添加
        if (!plugin)
        {
            KLOG_WARNING("The reinforcement %s is unsupported by any plugin.", reinforcementName.c_str());
            continue;
        }

        auto reinforcementNoArg = plugin->getReinforcementConfig(reinforcementName);
        if (!reinforcementNoArg)
        {
            KLOG_WARNING("The config of reinforcement %s is empty.", reinforcementName.c_str());
            continue;
        }

        // 添加加固项的基本信息（分类和标签）
        if (reinforcementNoArg->category().present())
        {
            reinforcementArg.category(reinforcementNoArg->category().get());
        }
        reinforcementArg.label(reinforcementNoArg->label());
        reinforcementArg.description(reinforcementNoArg->description());

        auto reinforcement = QSharedPointer<Reinforcement>(new Reinforcement(plugin->getId(), reinforcementArg));

        this->m_reinforcements[QString::fromStdString(reinforcementName)] = reinforcement;
    }

    Q_EMIT reinforcementsChanged();
}

void Plugins::joinReinforcement(Protocol::Reinforcement& destReinforcement,
                                const Protocol::Reinforcement& sourceReinforcement)
{
    KLOG_INFO() << "Join reinforcement" << sourceReinforcement.name().c_str();

    const auto& fromArgs = sourceReinforcement.arg();
    for (auto fromArgIter = fromArgs.begin(); fromArgIter != fromArgs.end(); ++fromArgIter)
    {
        auto& toArgs = destReinforcement.arg();
        for (auto toArgIter = toArgs.begin(); toArgIter != toArgs.end(); ++toArgIter)
        {
            CONTINUE_IF_TRUE(fromArgIter->name() != toArgIter->name());
            KLOG_INFO() << "Modify argument " << toArgIter->value().c_str() << " to " << fromArgIter->value().c_str();
            toArgIter->value(fromArgIter->value());
            break;
        }
    }
}

}  // namespace BR
}  // namespace KS
