/**
 * @file          /kiran-sse-manager/lib/core/sse-plugin-loader.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/sse-plugin-loader.h"

namespace Kiran
{
SSEPluginCPPLoader::SSEPluginCPPLoader(const std::string &so_path) : so_path_(so_path),
                                                                     is_activate_(false)
{
}

bool SSEPluginCPPLoader::load()
{
    return this->load_module();
}

bool SSEPluginCPPLoader::activate()
{
    KLOG_DEBUG("is activate: %d, so path: %s.", this->is_activate_, this->so_path_.c_str());

    // 不能重复激活
    RETURN_VAL_IF_TRUE(this->is_activate_, true);
    this->interface_->activate();
    return true;
}

bool SSEPluginCPPLoader::deactivate()
{
    KLOG_PROFILE("");

    // 未激活不能取消激活
    RETURN_VAL_IF_TRUE(!this->is_activate_, true);
    this->interface_->deactivate();
    return true;
}

bool SSEPluginCPPLoader::load_module()
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

        this->interface_ = std::shared_ptr<SSEPluginInterface>((Kiran::SSEPluginInterface *)((NewPluginFun)new_plugin_fun)(), (DelPluginFun)del_plugin_fun);
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

}  // namespace  Kiran
