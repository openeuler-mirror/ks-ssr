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

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "configuration.h"
#include "plugins.h"
#include "python/python-klog.h"
#include "utils.h"

namespace KS
{
namespace BRDaemon
{
#define PYTHON_CHECK_VERSION(major, minor, micro)                   \
    (PY_MAJOR_VERSION > (major) ||                                  \
     (PY_MAJOR_VERSION == (major) && PY_MINOR_VERSION > (minor)) || \
     (PY_MAJOR_VERSION == (major) && PY_MINOR_VERSION == (minor) && \
      PY_MICRO_VERSION >= (micro)))

Plugins::Plugins(Configuration* configuration) : configuration_(configuration),
                                                 thread_pool_(this->configuration_->getMaxThreadNum())
{
}

Plugins::~Plugins()
{
    Py_Finalize();
}

Plugins* Plugins::instance_ = NULL;
void Plugins::globalInit(Configuration* configuration)
{
    instance_ = new Plugins(configuration);
    instance_->init();
}

QSharedPointer<Plugin> Plugins::getPluginByReinforcement(const QString& name)
{
    auto iter = this->reinforcements_plugins_.find(name);
    RETURN_VAL_IF_TRUE(iter == this->reinforcements_plugins_.end(), QSharedPointer<Plugin>());
    return iter.value().lock();
}

BRReinforcementVec Plugins::getReinforcementsByCategory(const QString& category_name)
{
    BRReinforcementVec result;
    for (auto iter = this->reinforcements_.begin(); iter != this->reinforcements_.end(); ++iter)
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

    auto import_package_path = fmt::format("sys.path.append('{0}')", SSR_BR_PLUGIN_PYTHON_ROOT_DIR);
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
    PyRun_SimpleString(import_package_path.c_str());

    this->loadPlugins();
    this->loadReinforcements();

    // 这里对锁进行释放，确保其他线程可以获取到锁，如果主线程还需要操作Python解析器，则需要重新获取锁
    Utils::pyGiUnlock();

    QObject::connect(this->configuration_, &Configuration::rs_changed_, this, &Plugins::onRsChangedCb);
}

void Plugins::loadPlugins()
{
    KLOG_DEBUG("Plugins::loadPlugins");
    this->loadPluginsFromDir(SSR_BR_PLUGIN_CPP_ROOT_DIR);
    this->loadPluginsFromDir(SSR_BR_PLUGIN_PYTHON_ROOT_DIR);
}

void Plugins::loadPluginsFromDir(const QString& dirname)
{
    QDir plugin_dir(dirname);

    for (auto iter : plugin_dir.entryList(QDir::NoDotAndDotDot |
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
        auto plugin_loader = plugin->getLoader();
        if (!(plugin_loader->activate() && this->addPlugin(plugin)))
        {
            plugin_loader->deactivate();
        }
    }
}

bool Plugins::addPlugin(QSharedPointer<Plugin> plugin)
{
    RETURN_VAL_IF_FALSE(plugin, false);

    auto pluginId = plugin->getId();
    KLOG_DEBUG() << "plugin id: " << pluginId.toLatin1();

    if (this->plugins_.find(pluginId) != this->plugins_.end())
    {
        KLOG_WARNING() << "The plugin is already exist. id: %s." << pluginId;
        return false;
    }
    else
    {
        this->plugins_[pluginId] = plugin;
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
            this->reinforcements_plugins_[QString::fromStdString(reinforcement_name)] = plugin;
        }
    }
    return true;
}

void Plugins::loadReinforcements()
{
    KLOG_DEBUG("Plugins::loadReinforcements");

    // TODO: 更新优化，现有的加固项调用更新函数，这里更新后应该需要通过DBUS发送信号
    this->reinforcements_.clear();

    auto rs = this->configuration_->getRs();
    RETURN_IF_FALSE(rs);

    auto& reinforcements = rs->body().reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        auto& reinforcement_arg = (*iter);
        auto reinforcement_name = reinforcement_arg.name();
        auto plugin = this->getPluginByReinforcement(QString::fromStdString(reinforcement_name));

        // 加固标准中的加固项如果没有插件支持，则不添加
        if (!plugin)
        {
            KLOG_WARNING("The reinforcement %s is unsupported by any plugin.", reinforcement_name.c_str());
            continue;
        }

        auto reinforcement_noarg = plugin->getReinforcementConfig(reinforcement_name);
        if (!reinforcement_noarg)
        {
            KLOG_WARNING("The config of reinforcement %s is empty.", reinforcement_name.c_str());
            continue;
        }

        // 添加加固项的基本信息（分类和标签）
        if (reinforcement_noarg->category().present())
        {
            reinforcement_arg.category(reinforcement_noarg->category().get());
        }
        reinforcement_arg.label(reinforcement_noarg->label());
        reinforcement_arg.description(reinforcement_noarg->description());

        auto reinforcement = QSharedPointer<Reinforcement>(new Reinforcement(plugin->getId(), reinforcement_arg));

        this->reinforcements_[QString::fromStdString(reinforcement_name)] = reinforcement;
    }
}

void Plugins::onRsChangedCb()
{
    this->loadReinforcements();
}
}  // namespace BRDaemon
}  // namespace KS