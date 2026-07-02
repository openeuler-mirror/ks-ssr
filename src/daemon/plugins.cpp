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
#include "src/daemon/python/python-log.h"

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
    PyImport_AppendInittab("log", PyInit_log);

    auto import_package_path = fmt::format("sys.path.append('{0}')", SSR_PLUGIN_PYTHON_ROOT_DIR);
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(import_package_path.c_str());

    this->load_plugins();
    this->load_reinforcements();

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

    // TODO: 更新优化，现有的加固项调用更新函数
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