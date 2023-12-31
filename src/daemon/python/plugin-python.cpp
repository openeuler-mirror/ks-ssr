/**
 * @file          /ks-ssr-manager/src/daemon/python/plugin-python.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "src/daemon/python/plugin-python.h"
#include "src/daemon/utils.h"

namespace KS
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

    this->module_fullname_ = PyModule_GetName(this->module_);
    this->class_ = PyObject_GetAttrString(this->module_, this->class_name_.c_str());

    if (!this->class_ || !PyCallable_Check(this->class_))
    {
        KLOG_WARNING("Failed to get class %s.%s, class: %p, error: %s.",
                     this->module_fullname_.c_str(),
                     this->class_name_.c_str(),
                     this->class_,
                     Utils::py_catch_exception().c_str());
        return;
    }

    this->class_instance_ = PyObject_CallObject(this->class_, NULL);

    if (!this->class_instance_)
    {
        KLOG_WARNING("Failed to create object for class %s. error: %s.",
                     this->class_name_.c_str(),
                     Utils::py_catch_exception().c_str());
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

    auto gstate = PyGILState_Ensure();
#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->class_instance_, "get", NULL);
#else
    char method[] = "get";
    char *format = NULL;
    auto py_retval = PyObject_CallMethod(this->class_instance_, method, format);
#endif

    bool retval = true;

    do
    {
        if (!this->check_call_result(py_retval, this->class_name_ + ".get", error))
        {
            retval = false;
            break;
        }

        auto successed = PyTuple_GetItem(py_retval, 0);
        if (successed == Py_True)
        {
            args = Utils::pyobject_as_string(PyTuple_GetItem(py_retval, 1));
        }
        else
        {
            error = Utils::pyobject_as_string(PyTuple_GetItem(py_retval, 1));
        }
        retval = (successed == Py_True);
        break;
    } while (0);

    Py_XDECREF(py_retval);
    PyGILState_Release(gstate);

    return retval;
}

bool ReinforcementPython::set(const std::string &args, std::string &error)
{
    KLOG_DEBUG("Call set method in class %s.", this->class_name_.c_str());

    auto gstate = PyGILState_Ensure();

#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->class_instance_, "set", "(s)", args.c_str());
#else
    char method[] = "set";
    char format[] = "(s)";
    auto py_retval = PyObject_CallMethod(this->class_instance_, method, format, args.c_str());
#endif

    bool retval = true;
    do
    {
        if (!this->check_call_result(py_retval, this->class_name_ + ".set", error))
        {
            retval = false;
            break;
        }

        auto successed = PyTuple_GetItem(py_retval, 0);
        if (successed == Py_False)
        {
            error = Utils::pyobject_as_string(PyTuple_GetItem(py_retval, 1));
        }
        retval = (successed == Py_True);
        break;
    } while (0);

    Py_XDECREF(py_retval);
    PyGILState_Release(gstate);

    return retval;
}

bool ReinforcementPython::check_call_result(PyObject *py_retval, const std::string &function_name, std::string &error)
{
    error = Utils::py_catch_exception();
    RETURN_VAL_IF_FALSE(error.empty(), false);

    if (!py_retval || !PyTuple_Check(py_retval))
    {
        error = fmt::format(_("The return value of {0} isn't tuple type"), function_name);
        return false;
    }

    auto retval_num = PyTuple_Size(py_retval);
    if (int32_t(retval_num) < 2)
    {
        error = fmt::format(_("The number of tuple returned by {0} is less than 2."), function_name);
        return false;
    }

    // 参数1代表执行是否成功，如果成功，则参数2表示执行成功的结果，否则参数2表示执行失败的错误信息
    auto py_arg1 = PyTuple_GetItem(py_retval, 0);
    auto py_arg2 = PyTuple_GetItem(py_retval, 1);

    bool is_string = false;

#if PY_MAJOR_VERSION < 3
    is_string = PyString_Check(py_arg2);
#endif

    if (!PyBool_Check(py_arg1) || (!PyUnicode_Check(py_arg2) && !is_string))
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

    do
    {
        py_reinforcements = PyObject_GetAttrString(this->module_, PYTHON_PLUGIN_VAR_REINFORCEMENTS);
        if (!py_reinforcements || !PyTuple_Check(py_reinforcements))
        {
            KLOG_WARNING("Cannot find variable: %s.", PYTHON_PLUGIN_VAR_REINFORCEMENTS);
            break;
        }

        auto package_name = PyModule_GetName(this->module_);
        auto reinforcement_num = PyTuple_Size(py_reinforcements);

        KLOG_DEBUG("Package name: %s, reinforcement number: %d.", package_name, reinforcement_num);

        for (int32_t i = 0; i < int32_t(reinforcement_num); ++i)
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

                if (key == "name")
                {
                    reinforcement_name = value;
                }
                else if (key == "module")
                {
                    module_name = value;
                }
                else if (key == "class")
                {
                    class_name = value;
                }
                else
                {
                    KLOG_WARNING("Unknown key: %s.", key.c_str());
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
    } while (0);

    Py_XDECREF(py_reinforcements);
}

void PluginPython::deactivate()
{
    for (auto iter = this->reinforcements_modules_.begin(); iter != this->reinforcements_modules_.end(); ++iter)
    {
        Py_XDECREF(iter->second);
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
            KLOG_WARNING("Failed to load module: %s, error: %s.", module_fullname.c_str(), Utils::py_catch_exception().c_str());
            return;
        }
        this->reinforcements_modules_[module_fullname] = py_module;
    }

    auto reinforcement = std::make_shared<ReinforcementPython>(py_module, function_prefix);
    RETURN_IF_FALSE(reinforcement->is_valid());

    if (this->reinforcements_.find(reinforcement_name) != this->reinforcements_.end())
    {
        KLOG_WARNING("The reinforcement %s is repeated.", reinforcement_name.c_str());
        return;
    }
    else
    {
        this->reinforcements_[reinforcement_name] = reinforcement;
    }
}

}  // namespace Daemon
}  // namespace KS
