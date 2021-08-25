/**
 * @file          /kiran-ssr-manager/src/daemon/plugin-python.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "src/daemon/plugin-python.h"
#include "src/daemon/utils.h"

namespace Kiran
{
namespace Daemon
{
#define PYTHON_PLUGIN_VAR_REINFORCEMENTS "reinforcements"

#define PYTHON_REINFORCEMENT_FUNC_GET "r_get"
#define PYTHON_REINFORCEMENT_FUNC_SET "r_set"

ReinforcementPython::ReinforcementPython(const std::string &package_name,
                                         const std::string &module_name) : package_name_(package_name),
                                                                           module_name_(module_name),
                                                                           module_(NULL),
                                                                           get_method_(NULL),
                                                                           set_method_(NULL),
                                                                           valid_(false)
{
    auto module_fullname = fmt::format("{0}.{1}", this->package_name_, this->module_name_);
    this->module_ = PyImport_ImportModule(module_fullname.c_str());
    if (!this->module_)
    {
        KLOG_WARNING("Failed to load module: %s.", module_fullname.c_str());
        return;
    }

    this->get_method_ = PyObject_GetAttrString(this->module_, PYTHON_REINFORCEMENT_FUNC_GET);
    if (!this->get_method_ || !PyCallable_Check(this->get_method_))
    {
        KLOG_WARNING("Failed to get '%s' function.", PYTHON_REINFORCEMENT_FUNC_GET);
        return;
    }

    this->set_method_ = PyObject_GetAttrString(this->module_, PYTHON_REINFORCEMENT_FUNC_SET);
    if (!this->set_method_ || !PyCallable_Check(this->set_method_))
    {
        KLOG_WARNING("Failed to get '%s' function.", PYTHON_REINFORCEMENT_FUNC_SET);
        return;
    }
    this->valid_ = true;
}

ReinforcementPython::~ReinforcementPython()
{
    Py_XDECREF(this->module_);
}

bool ReinforcementPython::get(std::string &args, SSRErrorCode &error_code)
{
    auto py_retval = PyObject_CallObject(this->get_method_, NULL);
    SCOPE_EXIT({
        Py_XDECREF(py_retval);
    });

    if (!py_retval || !PyUnicode_Check(py_retval))
    {
        error_code = SSRErrorCode::ERROR_DAEMON_PLUGIN_CALL_PYTHON_FUNC_FAILED;
        return false;
    }

    args = Utils::PyUnicode_AsString(py_retval);
    return true;
}

bool ReinforcementPython::set(const std::string &args, SSRErrorCode &error_code)
{
    auto py_args = Py_BuildValue("(s)", args.c_str());
    SCOPE_EXIT({
        Py_XDECREF(py_args);
    });

    auto py_retval = PyObject_CallObject(this->get_method_, py_args);
    if (!py_retval || !PyBool_Check(py_retval))
    {
        error_code = SSRErrorCode::ERROR_DAEMON_PLUGIN_CALL_PYTHON_FUNC_FAILED;
        return false;
    }

    if (py_retval == Py_False)
    {
        error_code = SSRErrorCode::ERROR_DAEMON_PLUGIN_CALL_PYTHON_FUNC_FAILED;
        return false;
    }
    return true;
}

PluginPython::PluginPython(PyObject *module) : module_(module)
{
}

PluginPython::~PluginPython()
{
    Py_XDECREF(this->module_);
}

void PluginPython::activate()
{
    PyObject *reinforcements = NULL;
    PyObject *key = NULL;
    PyObject *value = NULL;
    Py_ssize_t pos = 0;

    SCOPE_EXIT({
        Py_XDECREF(reinforcements);
    });

    reinforcements = PyObject_GetAttrString(this->module_, PYTHON_PLUGIN_VAR_REINFORCEMENTS);
    if (!reinforcements || !PyDict_Check(reinforcements))
    {
        KLOG_WARNING("Cannot find variable: %s.", PYTHON_PLUGIN_VAR_REINFORCEMENTS);
        return;
    }

    auto package_name = PyModule_GetName(this->module_);
    while (PyDict_Next(reinforcements, &pos, &key, &value))
    {
        auto reinforcement_name = Utils::PyUnicode_AsString(key);
        auto module_name = Utils::PyUnicode_AsString(value);

        auto reinforcement = std::make_shared<ReinforcementPython>(package_name, module_name);
        CONTINUE_IF_FALSE(reinforcement->is_valid());

        auto iter = this->reinforcements_.emplace(reinforcement_name, reinforcement);
        if (!iter.second)
        {
            KLOG_WARNING("The reinforcement %s is repeated.", reinforcement_name);
        }
    }
}

void PluginPython::deactivate()
{
    this->reinforcements_.clear();
}

}  // namespace Daemon
}  // namespace Kiran
