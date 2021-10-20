/**
 * @file          /kiran-ssr-manager/src/daemon/plugins.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "src/daemon/configuration.h"
#include "src/daemon/plugins.h"
#include "src/daemon/python/python-klog.h"
#include "src/daemon/utils.h"

namespace Kiran
{
namespace Daemon
{
Plugins::Plugins(Configuration* configuration) : configuration_(configuration),
                                                 thread_pool_(this->configuration_->get_max_thread_num())
{
}

Plugins::~Plugins()
{
    Py_Finalize();
}

Plugins* Plugins::instance_ = nullptr;
void Plugins::global_init(Configuration* configuration)
{
    instance_ = new Plugins(configuration);
    instance_->init();
}

std::shared_ptr<Plugin> Plugins::get_plugin_by_reinforcement(const std::string& name)
{
    auto iter = this->reinforcements_plugins_.find(name);
    RETURN_VAL_IF_TRUE(iter == this->reinforcements_plugins_.end(), nullptr);
    return iter->second.lock();
}

SSRReinforcementVec Plugins::get_reinforcements_by_category(const std::string& category_name)
{
    SSRReinforcementVec result;
    for (auto iter : this->reinforcements_)
    {
        if (iter.second->get_category_name() == category_name)
        {
            result.push_back(iter.second);
        }
    }
    return result;
}

std::shared_ptr<SSRReinforcementInterface> Plugins::get_reinfocement_interface(const std::string& plugin_name,
                                                                               const std::string& reinforcement_name)
{
    auto plugin = this->get_plugin(plugin_name);
    if (!plugin)
    {
        KLOG_WARNING("Plugin '%s' of the reinforcement '%s' is not found.",
                     plugin_name.c_str(),
                     reinforcement_name.c_str());
        return nullptr;
    }

    auto plugin_interface = plugin->get_loader()->get_interface();
    if (!plugin_interface)
    {
        KLOG_WARNING("The Plugin interface for %s is nullptr.", plugin_name.c_str());
        return nullptr;
    }

    auto reinforcement_interface = plugin_interface->get_reinforcement(reinforcement_name);
    if (!reinforcement_interface)
    {
        KLOG_WARNING("The reinforcement interface for %s is nullptr.", reinforcement_name.c_str());
        return nullptr;
    }
    return reinforcement_interface;
}

void Plugins::init()
{
    // 内建模块的名称不支持package.module格式，因此这里不加ssr前缀了
    PyImport_AppendInittab("klog", PyInit_klog);

    auto import_package_path = fmt::format("sys.path.append('{0}')", SSR_PLUGIN_PYTHON_ROOT_DIR);
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
    PyEval_InitThreads();
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(import_package_path.c_str());

    this->load_plugins();
    this->load_reinforcements();

    // 这里对锁进行释放，确保其他线程可以获取到锁，如果主线程还需要操作Python解析器，则需要重新获取锁
    Utils::py_gi_unlock();

    this->configuration_->signal_rs_changed().connect(sigc::mem_fun(this, &Plugins::on_rs_changed_cb));
}

void Plugins::load_plugins()
{
    KLOG_PROFILE("");
    this->load_plugins_from_dir(SSR_PLUGIN_CPP_ROOT_DIR);
    this->load_plugins_from_dir(SSR_PLUGIN_PYTHON_ROOT_DIR);
}

void Plugins::load_plugins_from_dir(const std::string& dirname)
{
    try
    {
        Glib::Dir plugin_dir(dirname);
        for (auto iter = plugin_dir.begin(); iter != plugin_dir.end(); ++iter)
        {
            auto basename = *iter;
            auto filename = Glib::build_filename(dirname, basename);

            if (!Glib::file_test(filename, Glib::FILE_TEST_IS_REGULAR) ||
                !Glib::str_has_prefix(basename, "ssr-plugin"))
            {
                KLOG_DEBUG("Skip file %s.", filename.c_str());
                continue;
            }
            auto plugin = std::make_shared<Plugin>(filename);

            // 初始化->激活->添加插件
            if (plugin->init())
            {
                auto plugin_loader = plugin->get_loader();

                if (!plugin_loader->activate() || !this->add_plugin(plugin))
                {
                    plugin_loader->deactivate();
                }
            }
        }
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }
}

bool Plugins::add_plugin(std::shared_ptr<Plugin> plugin)
{
    RETURN_VAL_IF_FALSE(plugin, false);

    KLOG_DEBUG("plugin id: %s.", plugin->get_id().c_str());

    auto iter = this->plugins_.emplace(plugin->get_id(), plugin);
    if (!iter.second)
    {
        KLOG_WARNING("The plugin is already exist. id: %s.", plugin->get_id().c_str());
        return false;
    }

    auto reinforcement_names = plugin->get_reinforcement_names();
    for (auto reinforcement_name : reinforcement_names)
    {
        auto old_plugin = this->get_plugin_by_reinforcement(reinforcement_name);
        if (old_plugin)
        {
            KLOG_WARNING("The reinforcement %s is conflicted with other plugin. old plugin: %s, cur plugin: %s.",
                         reinforcement_name.c_str(),
                         old_plugin->get_id().c_str(),
                         plugin->get_id().c_str());
        }
        else
        {
            this->reinforcements_plugins_[reinforcement_name] = plugin;
        }
    }
    return true;
}

void Plugins::load_reinforcements()
{
    KLOG_PROFILE("");

    // TODO: 更新优化，现有的加固项调用更新函数，这里更新后应该需要通过DBUS发送信号
    this->reinforcements_.clear();

    auto rs = this->configuration_->get_rs();
    RETURN_IF_FALSE(rs);

    try
    {
        for (auto reinforcement_arg : rs->body().reinforcement())
        {
            auto reinforcement_name = reinforcement_arg.name();
            auto plugin = this->get_plugin_by_reinforcement(reinforcement_name);

            // 加固标准中的加固项如果没有插件支持，则不添加
            if (!plugin)
            {
                KLOG_WARNING("The reinforcement %s is unsupported by any plugin.", reinforcement_name.c_str());
                continue;
            }

            auto reinforcement_noarg = plugin->get_reinforcement_config(reinforcement_name);
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

            auto reinforcement = std::make_shared<Reinforcement>(plugin->get_id(),
                                                                 reinforcement_arg);

            this->reinforcements_.emplace(reinforcement_name, reinforcement);
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        this->reinforcements_.clear();
    }
}

void Plugins::on_rs_changed_cb()
{
    this->load_reinforcements();
}
}  // namespace Daemon
}  // namespace Kiran