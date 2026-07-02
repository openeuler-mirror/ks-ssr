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
                                         const std::string &class_name) : module_(module),
                                                                          class_name_(class_name),
                                                                          class_(NULL),
                                                                          class_instance_(NULL),
                                                                          valid_(false)
{
    Py_XINCREF(this->module_);

    this->class_ = PyObject_GetAttrString(this->module_, this->class_name_.c_str());
    if (!this->class_ || !PyCallable_Check(this->class_))
    {
        KLOG_WARNING("Failed to get class %s.%s. class: %p.", PyModule_GetName(module), this->class_name_.c_str(), this->class_);
        return;
    }

    this->class_instance_ = PyObject_CallObject(this->class_, NULL);

    if (!this->class_instance_)
    {
        KLOG_WARNING("Failed to create object for class %s. class instance: %p",
                     this->class_name_.c_str(),
                     this->class_instance_);
        return;
    }

    this->valid_ = true;
}

ReinforcementPython::~ReinforcementPython()
{
    Py_XDECREF(this->module_);
    Py_XDECREF(this->class_);
}

bool ReinforcementPython::get(std::string &args, std::string &error)
{
    KLOG_DEBUG("Call get method in class %s.", this->class_name_.c_str());

#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->class_instance_, "get", NULL);
#else
    char method[] = "get";
    char *format = NULL;
    auto py_retval = PyObject_CallMethod(this->class_instance_, method, format);
#endif

    SCOPE_EXIT({
        Py_XDECREF(py_retval);
    });

    RETURN_VAL_IF_FALSE(this->check_call_result(py_retval, this->class_name_ + ".get", error), false);

    auto successed = PyTuple_GetItem(py_retval, 0);
    if (successed == Py_True)
    {
        args = Utils::pyobject_as_string(PyTuple_GetItem(py_retval, 1));
        return true;
    }
    else
    {
        error = Utils::pyobject_as_string(PyTuple_GetItem(py_retval, 1));
        return false;
    }
}

bool ReinforcementPython::set(const std::string &args, std::string &error)
{
    KLOG_DEBUG("Call set method in class %s.", this->class_name_.c_str());

#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->class_instance_, "set", "(s)", args.c_str());
#else
    char method[] = "set";
    char format[] = "(s)";
    auto py_retval = PyObject_CallMethod(this->class_instance_, method, format, args.c_str());
#endif
    RETURN_VAL_IF_FALSE(this->check_call_result(py_retval, this->class_name_ + ".set", error), false);

    auto successed = PyTuple_GetItem(py_retval, 0);
    if (successed == Py_False)
    {
        error = Utils::pyobject_as_string(PyTuple_GetItem(py_retval, 1));
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

    if (!PyBool_Check(py_arg1) || (!PyUnicode_Check(py_arg2) && !PyString_Check(py_arg2)))
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
        std::string class_name;
        PyObject *py_key = NULL;
        PyObject *py_value = NULL;
        Py_ssize_t py_pos = 0;

        while (PyDict_Next(py_reinforcement, &py_pos, &py_key, &py_value))
        {
            auto key = Utils::pyobject_as_string(py_key);
            auto value = Utils::pyobject_as_string(py_value);

            KLOG_DEBUG("key: %s, value: %s.", key.c_str(), value.c_str());

            switch (shash(key.c_str()))
            {
            case "name"_hash:
                reinforcement_name = value;
                break;
            case "module"_hash:
                module_name = value;
                break;
            case "class"_hash:
                class_name = value;
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
        CHECK_KEY_NOT_EMPTY(class_name, class)

#undef CHECK_KEY_NOT_EMPTY

        this->add_reinforcement(package_name, module_name, reinforcement_name, class_name);
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
            KLOG_WARNING("Failed to load module: %s, error: %s.", module_fullname.c_str(), Utils::catch_exception().c_str());
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
