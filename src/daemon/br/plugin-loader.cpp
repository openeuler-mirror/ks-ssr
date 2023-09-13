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

#include "plugin-loader.h"
#include "python/plugin-python.h"
#include "utils.h"

namespace KS
{
namespace BRDaemon
{
PluginCPPLoader::PluginCPPLoader(const QString &so_path) : so_path_(so_path),
                                                           is_activate_(false)
{
}

bool PluginCPPLoader::load()
{
    return this->load_module();
}

bool PluginCPPLoader::activate()
{
    KLOG_DEBUG("is activate: %d, so path: %s.", this->is_activate_, this->so_path_.toLatin1());

    // 不能重复激活
    RETURN_VAL_IF_TRUE(this->is_activate_, true);
    this->interface_->activate();
    return true;
}

bool PluginCPPLoader::deactivate()
{
    KLOG_DEBUG("");
    // 未激活不能取消激活
    RETURN_VAL_IF_TRUE(!this->is_activate_, true);
    this->interface_->deactivate();
    return true;
}

bool PluginCPPLoader::load_module()
{
    KLOG_DEBUG("load module %s", this->so_path_.toLatin1());

    this->module_ = QSharedPointer<QLibrary>(new QLibrary(this->so_path_));

    if (this->module_->load())
    {
        QFunctionPointer new_plugin_fun = nullptr;
        QFunctionPointer del_plugin_fun = nullptr;

        if ((new_plugin_fun = this->module_->resolve("new_plugin")) != nullptr)
        {
            KLOG_WARNING("Not found function 'new_plugin' in module %s.", this->so_path_.toLatin1());
            return false;
        }

        if ((del_plugin_fun = this->module_->resolve("delete_plugin")) != nullptr)
        {
            KLOG_WARNING("not found function 'delete_plugin' in module %s.", this->so_path_.toLatin1());
            return false;
        }

        this->interface_ = QSharedPointer<BRPluginInterface>((KS::BRPluginInterface *)((NewPluginFun)new_plugin_fun)(), (DelPluginFun)del_plugin_fun);
        return true;
    }
    else
    {
        KLOG_WARNING("open module %s fail: %s.",
                     this->so_path_.toLatin1(),
                     this->module_ ? this->module_->errorString().toLatin1() : "unknown");
        return false;
    }

    return true;
}

PluginPythonLoader::PluginPythonLoader(const QString &package_name) : package_name_(package_name),
                                                                      is_activate_(false)
{
}

bool PluginPythonLoader::load()
{
    auto module = PyImport_ImportModule(this->package_name_.toLatin1());
    bool retval = true;

    do
    {
        if (!module)
        {
            KLOG_WARNING("Failed to load module: %s, error: %s.",
                         this->package_name_.toLatin1(),
                         Utils::pyCatchException().toLatin1());
            retval = false;
            break;
        }
        this->interface_ = QSharedPointer<PluginPython>(new PluginPython(module));
    } while (0);

    Py_XDECREF(module);
    return retval;
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
    KLOG_DEBUG("");
    // 未激活不能取消激活
    RETURN_VAL_IF_TRUE(!this->is_activate_, true);
    this->interface_->deactivate();
    return true;
}
}  // namespace BRDaemon
}  // namespace KS
