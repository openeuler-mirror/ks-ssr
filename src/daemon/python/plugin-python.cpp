/**
 * @file          /kiran-ssr-manager/src/daemon/python/plugin-python.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "src/daemon/python/plugin-python.h"
#include "src/daemon/utils.h"

namespace Kiran
{
namespace Daemon
{
#define PYTHON_PLUGIN_VAR_REINFORCEMENTS "reinforcements"

ReinforcementPython::ReinforcementPython(PyObject *module,
                                         const std::string &function_prefix) : module_(module),
                                                                               function_prefix_(function_prefix),
                                                                               get_method_(NULL),
                                                                               set_method_(NULL),
                                                                               valid_(false)
{
    Py_XINCREF(this->module_);

    this->get_method_name_ = fmt::format("{0}_get", function_prefix);
    this->set_method_name_ = fmt::format("{0}_set", function_prefix);

    this->get_method_ = PyObject_GetAttrString(this->module_, this->get_method_name_.c_str());
    if (!this->get_method_ || !PyCallable_Check(this->get_method_))
    {
        KLOG_WARNING("Failed to get '%s' function.", this->get_method_name_.c_str());
        return;
    }

    this->set_method_ = PyObject_GetAttrString(this->module_, this->set_method_name_.c_str());
    if (!this->set_method_ || !PyCallable_Check(this->set_method_))
    {
        KLOG_WARNING("Failed to get '%s' function.", this->set_method_name_.c_str());
        return;
    }
    this->valid_ = true;
}

ReinforcementPython::~ReinforcementPython()
{
    Py_XDECREF(this->module_);
}

bool ReinforcementPython::get(std::string &args, std::string &error)
{
    KLOG_DEBUG("Call method %s. ", this->get_method_name_.c_str());

    auto py_retval = PyObject_CallObject(this->get_method_, NULL);
    SCOPE_EXIT({
        Py_XDECREF(py_retval);
    });

    RETURN_VAL_IF_FALSE(this->check_call_result(py_retval, this->get_method_name_, error), false);

    auto successed = PyTuple_GetItem(py_retval, 0);
    if (successed == Py_True)
    {
        args = Utils::PyUnicode_AsString(PyTuple_GetItem(py_retval, 1));
        return true;
    }
    else
    {
        error = Utils::PyUnicode_AsString(PyTuple_GetItem(py_retval, 1));
        return false;
    }
}

bool ReinforcementPython::set(const std::string &args, std::string &error)
{
    KLOG_DEBUG("Call method %s. ", this->set_method_name_.c_str());

    auto py_args = Py_BuildValue("(s)", args.c_str());
    SCOPE_EXIT({
        Py_XDECREF(py_args);
    });

    auto py_retval = PyObject_CallObject(this->set_method_, py_args);

    RETURN_VAL_IF_FALSE(this->check_call_result(py_retval, this->set_method_name_, error), false);

    auto successed = PyTuple_GetItem(py_retval, 0);
    if (successed == Py_False)
    {
        error = Utils::PyUnicode_AsString(PyTuple_GetItem(py_retval, 1));
        return false;
    }

    return true;
}

bool ReinforcementPython::check_call_result(PyObject *py_retval, const std::string &function_name, std::string &error)
{
    error = Utils::catch_exception();
    RETURN_VAL_IF_FALSE(error.empty(), false);

    if (!py_retval || !PyTuple_Check(py_retval))
    {
        error = fmt::format(_("The return value of {0} isn't tuple type"), function_name);
        return false;
    }

    auto retval_num = PyTuple_Size(py_retval);
    if (retval_num < 2)
    {
        error = fmt::format(_("The number of tuple returned by {0} is less than 2."), function_name);
        return false;
    }

    // 参数1代表执行是否成功，如果成功，则参数2表示执行成功的结果，否则参数2表示执行失败的错误信息
    auto py_arg1 = PyTuple_GetItem(py_retval, 0);
    auto py_arg2 = PyTuple_GetItem(py_retval, 1);

    if (!PyBool_Check(py_arg1) || !PyUnicode_Check(py_arg2))
    {
        error = fmt::format(_("The type of tuple item returned by {0} is invalid."), function_name);
        return false;
    }

    return true;
}

PluginPython::PluginPython(PyObject *module) : module_(module)
{
    Py_XINCREF(this->module_);
}

PluginPython::~PluginPython()
{
    this->deactivate();
    Py_XDECREF(this->module_);
}

void PluginPython::activate()
{
    PyObject *py_reinforcements = NULL;

    SCOPE_EXIT({
        Py_XDECREF(py_reinforcements);
    });

    py_reinforcements = PyObject_GetAttrString(this->module_, PYTHON_PLUGIN_VAR_REINFORCEMENTS);
    if (!py_reinforcements || !PyTuple_Check(py_reinforcements))
    {
        KLOG_WARNING("Cannot find variable: %s.", PYTHON_PLUGIN_VAR_REINFORCEMENTS);
        return;
    }

    auto package_name = PyModule_GetName(this->module_);
    auto reinforcement_num = PyTuple_Size(py_reinforcements);

    KLOG_DEBUG("Package name: %s, reinforcement number: %d.", package_name, reinforcement_num);

    for (uint32_t i = 0; i < reinforcement_num; ++i)
    {
        auto py_reinforcement = PyTuple_GetItem(py_reinforcements, i);
        if (!py_reinforcement || !PyDict_Check(py_reinforcement))
        {
            KLOG_WARNING("The %d-th item of reinforcements isn't dict type.", i);
            continue;
        }

        std::string reinforcement_name;
        std::string module_name;
        std::string function_prefix;
        PyObject *py_key = NULL;
        PyObject *py_value = NULL;
        Py_ssize_t py_pos = 0;

        while (PyDict_Next(py_reinforcement, &py_pos, &py_key, &py_value))
        {
            auto key = Utils::PyUnicode_AsString(py_key);
            auto value = Utils::PyUnicode_AsString(py_value);

            KLOG_DEBUG("key: %s, value: %s.", key.c_str(), value.c_str());

            switch (shash(key.c_str()))
            {
            case "name"_hash:
                reinforcement_name = value;
                break;
            case "module"_hash:
                module_name = value;
                break;
            case "function_prefix"_hash:
                function_prefix = value;
                break;
            default:
                KLOG_WARNING("Unknown key: %s.", key.c_str());
                break;
            }
        }

#define CHECK_KEY_NOT_EMPTY(var, key)               \
    if (var.empty())                                \
    {                                               \
        KLOG_WARNING("The %s is not found.", #key); \
        continue;                                   \
    }

        CHECK_KEY_NOT_EMPTY(reinforcement_name, name)
        CHECK_KEY_NOT_EMPTY(module_name, module)
        CHECK_KEY_NOT_EMPTY(function_prefix, function_prefix)

#undef CHECK_KEY_NOT_EMPTY

        this->add_reinforcement(package_name, module_name, reinforcement_name, function_prefix);
    }
}

void PluginPython::deactivate()
{
    for (auto iter : this->reinforcements_modules_)
    {
        Py_XDECREF(iter.second);
    }
    this->reinforcements_.clear();
    this->reinforcements_modules_.clear();
}

void PluginPython::add_reinforcement(const std::string &package_name,
                                     const std::string &module_name,
                                     const std::string &reinforcement_name,
                                     const std::string &function_prefix)
{
    auto module_fullname = fmt::format("{0}.{1}", package_name, module_name);

    auto py_module = this->get_reinforcement_module(module_fullname);
    if (!py_module)
    {
        py_module = PyImport_ImportModule(module_fullname.c_str());
        if (!py_module)
        {
            KLOG_WARNING("Failed to load module: %s.", module_fullname.c_str());
            return;
        }
        this->reinforcements_modules_.emplace(module_fullname, py_module);
    }

    auto reinforcement = std::make_shared<ReinforcementPython>(py_module, function_prefix);
    RETURN_IF_FALSE(reinforcement->is_valid());

    auto iter = this->reinforcements_.emplace(reinforcement_name, reinforcement);
    if (!iter.second)
    {
        KLOG_WARNING("The reinforcement %s is repeated.", reinforcement_name);
        return;
    }
}

}  // namespace Daemon
}  // namespace Kiran
