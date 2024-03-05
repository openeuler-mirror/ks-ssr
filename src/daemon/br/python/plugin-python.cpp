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

#include "plugin-python.h"
#include "src/daemon/br/utils.h"

namespace KS
{
namespace BRDaemon
{
#define PYTHON_PLUGIN_VAR_REINFORCEMENTS "reinforcements"

ReinforcementPython::ReinforcementPython(PyObject *module,
                                         const QString &class_name)
    : module_(module),
      class_name_(class_name),
      class_(NULL),
      class_instance_(NULL),
      valid_(false)
{
    Py_XINCREF(this->module_);

    this->module_fullname_ = PyModule_GetName(this->module_);
    this->class_ = PyObject_GetAttrString(this->module_, this->class_name_.toLocal8Bit());

    if (!this->class_ || !PyCallable_Check(this->class_))
    {
        KLOG_WARNING() << "Failed to get class "
                       << this->module_fullname_.toLocal8Bit()
                       << "."
                       << this->class_name_.toLocal8Bit()
                       << ", class: "
                       << this->class_
                       << ", error: "
                       << Utils::pyCatchException().toLocal8Bit();
        return;
    }

    this->class_instance_ = PyObject_CallObject(this->class_, NULL);

    if (!this->class_instance_)
    {
        KLOG_WARNING() << "Failed to create object for class: "
                       << this->class_name_.toLocal8Bit()
                       << ", error: "
                       << Utils::pyCatchException().toLocal8Bit();
        return;
    }

    this->valid_ = true;
}

ReinforcementPython::~ReinforcementPython()
{
    Py_XDECREF(this->module_);
    Py_XDECREF(this->class_);
}

bool ReinforcementPython::get(QString &args, QString &error)
{
    KLOG_DEBUG("Call get method in class %s.", this->class_name_.toLocal8Bit().data());
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
            args = Utils::pyobjectAsString(PyTuple_GetItem(py_retval, 1));
        }
        else
        {
            error = Utils::pyobjectAsString(PyTuple_GetItem(py_retval, 1));
        }
        KLOG_DEBUG() << "args = " << args << "error = " << error;
        retval = (successed == Py_True);
        break;
    } while (0);

    Py_XDECREF(py_retval);
    PyGILState_Release(gstate);

    return retval;
}

bool ReinforcementPython::set(const QString &args, QString &error)
{
    auto args_toLocal8Bit = args.toLocal8Bit();
    KLOG_DEBUG("Call set method in class %s.", this->class_name_.toLocal8Bit().data());
    KLOG_DEBUG("args is %s.", args_toLocal8Bit.data());

    auto gstate = PyGILState_Ensure();

#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->class_instance_, "set", "(s)", args_toLocal8Bit.data());
#else
    char method[] = "set";
    char format[] = "(s)";
    auto py_retval = PyObject_CallMethod(this->class_instance_, method, format, args_toLocal8Bit.data());
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
            error = Utils::pyobjectAsString(PyTuple_GetItem(py_retval, 1));
        }
        retval = (successed == Py_True);
        break;
    } while (0);

    Py_XDECREF(py_retval);
    PyGILState_Release(gstate);

    return retval;
}

bool ReinforcementPython::check_call_result(PyObject *py_retval, const QString &function_name, QString &error)
{
    error = Utils::pyCatchException();
    if (!error.isEmpty())
    {
        KLOG_DEBUG() << "check result error, and error message is " << error << " function name is " << function_name;
        return false;
    }

    if (!py_retval || !PyTuple_Check(py_retval))
    {
        error = QString(QObject::tr("The return value of %1 isn't tuple type")).arg(function_name);
        // error = fmt::format(_("The return value of {0} isn't tuple type"), function_name);
        return false;
    }

    auto retval_num = PyTuple_Size(py_retval);
    if (int32_t(retval_num) < 2)
    {
        error = QString(QObject::tr("The number of tuple returned by %1 is less than 2.")).arg(function_name);
        // error = fmt::format(_("The number of tuple returned by {0} is less than 2."), function_name);
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
        error = QString(QObject::tr("The type of tuple item returned by %1 is invalid.")).arg(function_name);
        // error = fmt::format(_("The type of tuple item returned by {0} is invalid."), function_name);
        return false;
    }

    return true;
}

PluginPython::PluginPython(PyObject *module)
    : module_(module)
{
    Py_XINCREF(this->module_);
}

PluginPython::~PluginPython()
{
    // 在运行时多态的场景下，this->deactivate 会调用到基类的 deactivate ，而不是子类的重载后的 deactivate ，导致出现问题。
    this->clean();
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

        KLOG_DEBUG() << "Package name: " << package_name << ", reinforcement number: " << reinforcement_num;

        for (int32_t i = 0; i < int32_t(reinforcement_num); ++i)
        {
            auto py_reinforcement = PyTuple_GetItem(py_reinforcements, i);
            if (!py_reinforcement || !PyDict_Check(py_reinforcement))
            {
                KLOG_WARNING("The %d-th item of reinforcements isn't dict type.", i);
                continue;
            }

            QString reinforcement_name;
            QString module_name;
            QString class_name;
            PyObject *py_key = NULL;
            PyObject *py_value = NULL;
            Py_ssize_t py_pos = 0;

            while (PyDict_Next(py_reinforcement, &py_pos, &py_key, &py_value))
            {
                auto key = Utils::pyobjectAsString(py_key);
                auto value = Utils::pyobjectAsString(py_value);

                KLOG_DEBUG("key: %s, value: %s.", key.toLocal8Bit().data(), value.toLocal8Bit().data());

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
                    KLOG_WARNING() << "Unknown key: " << key.toLatin1();
                }
            }

#define CHECK_KEY_NOT_EMPTY(var, key)               \
    if (var.isEmpty())                              \
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
    clean();
}

void PluginPython::clean()
{
    for (auto iter = this->reinforcements_modules_.begin(); iter != this->reinforcements_modules_.end(); ++iter)
    {
        Py_XDECREF(iter.value());
    }
    this->reinforcements_.clear();
    this->reinforcements_modules_.clear();
}

void PluginPython::add_reinforcement(const QString &package_name,
                                     const QString &module_name,
                                     const QString &reinforcement_name,
                                     const QString &function_prefix)
{
    // auto module_fullname = fmt::format("{0}.{1}", package_name, module_name);
    auto module_fullname = QString("%1.%2").arg(package_name, module_name);

    auto py_module = this->get_reinforcement_module(module_fullname);
    if (!py_module)
    {
        py_module = PyImport_ImportModule(module_fullname.toLatin1());
        if (!py_module)
        {
            KLOG_WARNING() << "Failed to load module: " << module_fullname.toLatin1() << ", error: " << Utils::pyCatchException().toLatin1() << ".";
            return;
        }
        this->reinforcements_modules_[module_fullname] = py_module;
    }

    auto reinforcement = QSharedPointer<ReinforcementPython>(new ReinforcementPython(py_module, function_prefix));
    RETURN_IF_FALSE(reinforcement->isValid());

    if (this->reinforcements_.find(reinforcement_name) != this->reinforcements_.end())
    {
        KLOG_WARNING() << "The reinforcement " << reinforcement_name.toLatin1() << " is repeated.";
        return;
    }
    else
    {
        this->reinforcements_[reinforcement_name] = reinforcement;
    }
}

}  // namespace BRDaemon
}  // namespace KS
