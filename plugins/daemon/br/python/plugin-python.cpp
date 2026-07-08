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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "plugin-python.h"
#include "utils.h"

namespace KS
{
namespace BR
{
#define PYTHON_PLUGIN_VAR_REINFORCEMENTS "reinforcements"

ReinforcementPython::ReinforcementPython(PyObject *module,
                                         const QString &class_name)
    : m_module(module),
      m_className(class_name),
      m_class(NULL),
      m_classInstance(NULL),
      m_valid(false)
{
    Py_XINCREF(this->m_module);

    this->m_moduleFullname = PyModule_GetName(this->m_module);
    this->m_class = PyObject_GetAttrString(this->m_module, this->m_className.toLocal8Bit());

    if (!this->m_class || !PyCallable_Check(this->m_class))
    {
        KLOG_WARNING() << "Failed to get class "
                       << this->m_moduleFullname.toLocal8Bit()
                       << "."
                       << this->m_className.toLocal8Bit()
                       << ", class: "
                       << this->m_class
                       << ", error: "
                       << Utils::pyCatchException().toLocal8Bit();
        return;
    }

    this->m_classInstance = PyObject_CallObject(this->m_class, NULL);

    if (!this->m_classInstance)
    {
        KLOG_WARNING() << "Failed to create object for class: "
                       << this->m_className.toLocal8Bit()
                       << ", error: "
                       << Utils::pyCatchException().toLocal8Bit();
        return;
    }

    this->m_valid = true;
}

ReinforcementPython::~ReinforcementPython()
{
    Py_XDECREF(this->m_module);
    Py_XDECREF(this->m_class);
}

bool ReinforcementPython::get(QString &args, QString &error)
{
    KLOG_DEBUG("Call get method in class %s.", this->m_className.toLocal8Bit().data());
    auto gstate = PyGILState_Ensure();
#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->m_classInstance, "get", NULL);
#else
    char method[] = "get";
    char *format = NULL;
    auto py_retval = PyObject_CallMethod(this->class_instance_, method, format);
#endif

    bool retval = true;

    do
    {
        if (!this->checkCallResult(py_retval, this->m_className + ".get", error))
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
    KLOG_DEBUG("Call set method in class %s.", this->m_className.toLocal8Bit().data());
    KLOG_DEBUG("args is %s.", args_toLocal8Bit.data());

    auto gstate = PyGILState_Ensure();

#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->m_classInstance, "set", "(s)", args_toLocal8Bit.data());
#else
    char method[] = "set";
    char format[] = "(s)";
    auto py_retval = PyObject_CallMethod(this->class_instance_, method, format, args_toLocal8Bit.data());
#endif

    bool retval = true;
    do
    {
        if (!this->checkCallResult(py_retval, this->m_className + ".set", error))
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

bool ReinforcementPython::checkCallResult(PyObject *pyRetval, const QString &functionName, QString &error)
{
    error = Utils::pyCatchException();
    if (!error.isEmpty())
    {
        KLOG_DEBUG() << "check result error, and error message is " << error << " function name is " << functionName;
        return false;
    }

    if (!pyRetval || !PyTuple_Check(pyRetval))
    {
        error = QString(QObject::tr("The return value of %1 isn't tuple type.")).arg(functionName);
        // error = fmt::format(_("The return value of {0} isn't tuple type"), function_name);
        return false;
    }

    auto retvalNum = PyTuple_Size(pyRetval);
    if (int32_t(retvalNum) < 2)
    {
        error = QString(QObject::tr("The number of tuple returned by %1 is less than 2.")).arg(functionName);
        // error = fmt::format(_("The number of tuple returned by {0} is less than 2."), function_name);
        return false;
    }

    // 参数1代表执行是否成功，如果成功，则参数2表示执行成功的结果，否则参数2表示执行失败的错误信息
    auto pyArg1 = PyTuple_GetItem(pyRetval, 0);
    auto pyArg2 = PyTuple_GetItem(pyRetval, 1);

    bool is_string = false;

#if PY_MAJOR_VERSION < 3
    is_string = PyString_Check(py_arg2);
#endif

    if (!PyBool_Check(pyArg1) || (!PyUnicode_Check(pyArg2) && !is_string))
    {
        error = QString(QObject::tr("The type of tuple item returned by %1 is invalid.")).arg(functionName);
        // error = fmt::format(_("The type of tuple item returned by {0} is invalid."), function_name);
        return false;
    }

    return true;
}

PluginPython::PluginPython(PyObject *module)
    : m_module(module)
{
    Py_XINCREF(this->m_module);
}

PluginPython::~PluginPython()
{
    // 在运行时多态的场景下，this->deactivate 会调用到基类的 deactivate ，而不是子类的重载后的 deactivate ，导致出现问题。
    this->clean();
    Py_XDECREF(this->m_module);
}

void PluginPython::activate()
{
    PyObject *pyReinforcements = NULL;

    do
    {
        pyReinforcements = PyObject_GetAttrString(this->m_module, PYTHON_PLUGIN_VAR_REINFORCEMENTS);
        if (!pyReinforcements || !PyTuple_Check(pyReinforcements))
        {
            KLOG_WARNING("Cannot find variable: %s.", PYTHON_PLUGIN_VAR_REINFORCEMENTS);
            break;
        }

        auto packageName = PyModule_GetName(this->m_module);
        auto reinforcementNum = PyTuple_Size(pyReinforcements);

        KLOG_DEBUG() << "Package name: " << packageName << ", reinforcement number: " << reinforcementNum;

        for (int32_t i = 0; i < int32_t(reinforcementNum); ++i)
        {
            auto pyReinforcement = PyTuple_GetItem(pyReinforcements, i);
            if (!pyReinforcement || !PyDict_Check(pyReinforcement))
            {
                KLOG_WARNING("The %d-th item of reinforcements isn't dict type.", i);
                continue;
            }

            QString reinforcementName;
            QString moduleName;
            QString className;
            PyObject *pyKey = NULL;
            PyObject *pyValue = NULL;
            Py_ssize_t pyPos = 0;

            while (PyDict_Next(pyReinforcement, &pyPos, &pyKey, &pyValue))
            {
                auto key = Utils::pyobjectAsString(pyKey);
                auto value = Utils::pyobjectAsString(pyValue);

                KLOG_DEBUG("key: %s, value: %s.", key.toLocal8Bit().data(), value.toLocal8Bit().data());

                if (key == "name")
                {
                    reinforcementName = value;
                }
                else if (key == "module")
                {
                    moduleName = value;
                }
                else if (key == "class")
                {
                    className = value;
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

            CHECK_KEY_NOT_EMPTY(reinforcementName, name)
            CHECK_KEY_NOT_EMPTY(moduleName, module)
            CHECK_KEY_NOT_EMPTY(className, class)

#undef CHECK_KEY_NOT_EMPTY

            this->addReinforcement(packageName, moduleName, reinforcementName, className);
        }
    } while (0);

    Py_XDECREF(pyReinforcements);
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

}  // namespace BR
}  // namespace KS
