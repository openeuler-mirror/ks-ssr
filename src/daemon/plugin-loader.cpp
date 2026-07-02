/**
 * @file          /kiran-ssr-manager/src/daemon/plugin-loader.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "src/daemon/plugin-loader.h"
#include "src/daemon/python/plugin-python.h"

namespace Kiran
{
namespace Daemon
{
PluginCPPLoader::PluginCPPLoader(const std::string &so_path) : so_path_(so_path),
                                                               is_activate_(false)
{
}

bool PluginCPPLoader::load()
{
    return this->load_module();
}

bool PluginCPPLoader::activate()
{
    KLOG_DEBUG("is activate: %d, so path: %s.", this->is_activate_, this->so_path_.c_str());

    // 不能重复激活
    RETURN_VAL_IF_TRUE(this->is_activate_, true);
    this->interface_->activate();
    return true;
}

bool PluginCPPLoader::deactivate()
{
    KLOG_PROFILE("");
    // 未激活不能取消激活
    RETURN_VAL_IF_TRUE(!this->is_activate_, true);
    this->interface_->deactivate();
    return true;
}

bool PluginCPPLoader::load_module()
{
    KLOG_PROFILE("load module %s", this->so_path_.c_str());

    this->module_ = std::make_shared<Glib::Module>(this->so_path_);

    if (this->module_ && (*this->module_))
    {
        void *new_plugin_fun = nullptr;
        void *del_plugin_fun = nullptr;

        if (!this->module_->get_symbol("new_plugin", new_plugin_fun))
        {
            KLOG_WARNING("Not found function 'new_plugin' in module %s.", this->so_path_.c_str());
            return false;
        }

        if (!this->module_->get_symbol("delete_plugin", del_plugin_fun))
        {
            KLOG_WARNING("not found function 'delete_plugin' in module %s.", this->so_path_.c_str());
            return false;
        }

        this->interface_ = std::shared_ptr<SSRPluginInterface>((Kiran::SSRPluginInterface *)((NewPluginFun)new_plugin_fun)(), (DelPluginFun)del_plugin_fun);
        return true;
    }
    else
    {
        KLOG_WARNING("open module %s fail: %s.",
                     this->so_path_.c_str(),
                     this->module_ ? this->module_->get_last_error().c_str() : "unknown");
        return false;
    }

    return true;
}

PluginPythonLoader::PluginPythonLoader(const std::string &package_name) : package_name_(package_name),
                                                                          is_activate_(false)
{
}

bool PluginPythonLoader::load()
{
    auto module = PyImport_ImportModule(this->package_name_.c_str());
    SCOPE_EXIT({
        Py_XDECREF(module);
    });

    if (!module)
    {
        KLOG_WARNING("Failed to load module: %s.", this->package_name_.c_str());
        return false;
    }

    this->interface_ = std::make_shared<PluginPython>(module);
    return true;
}

bool PluginPythonLoader::activate()
{
    KLOG_DEBUG("is activate: %d", this->is_activate_);
    // 不能重复激活
    RETURN_VAL_IF_TRUE(this->is_activate_, true);
    this->interface_->activate();
    return true;
}

bool PluginPythonLoader::deactivate()
{
    KLOG_PROFILE("");
    // 未激活不能取消激活
    RETURN_VAL_IF_TRUE(!this->is_activate_, true);
    this->interface_->deactivate();
    return true;
}
}  // namespace Daemon
}  // namespace  Kiran
