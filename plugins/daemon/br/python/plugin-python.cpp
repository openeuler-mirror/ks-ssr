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
#include <QCoreApplication>

#include "plugin-python.h"
#include "utils.h"

namespace KS
{
namespace BR
{
#define PYTHON_PLUGIN_VAR_REINFORCEMENTS "reinforcements"

ReinforcementPython::ReinforcementPython(PyObject *module,
                                         const QString &className)
    : m_module(module),
      m_className(className),
      m_class(NULL),
      m_classInstance(NULL),
      m_isInited(false)
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

    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Device busy, please pop up!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Please contact the admin."));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop service!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Abnormal service!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Please close SELinux and use it!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "No such file or directory."));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Failed to execute command. Please check the log information for details."));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "UsePAM is not recommended to be closed,\nwhich will cause many problems!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop firewalld service!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop bluetooth service!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop cups service!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop avahi service!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop rpcbind service!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop smb service!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "sshd.services is not running!"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Abnormal service! Please check the log information for details."));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "No related services found"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "PAM is not configured with a faillock, please manually configure it"));
}

ReinforcementPython::~ReinforcementPython()
{
    Py_XDECREF(this->m_module);
    Py_XDECREF(this->m_class);
}

bool ReinforcementPython::init()
{
    RETURN_VAL_IF_TRUE(isInit(), true);

    this->m_classInstance = PyObject_CallObject(this->m_class, NULL);

    if (!this->m_classInstance)
    {
        KLOG_WARNING() << "Failed to create object for class: "
                       << this->m_className.toLocal8Bit()
                       << ", error: "
                       << Utils::pyCatchException().toLocal8Bit();
        return false;
    }

    m_isInited = true;
    return true;
}

bool ReinforcementPython::isInit()
{
    return m_isInited;
}

bool ReinforcementPython::get(QString &args, QString &error)
{
    auto gstate = PyGILState_Ensure();
#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->m_classInstance, "get", NULL);
#else
    char method[] = "get";
    char *format = NULL;
    auto py_retval = PyObject_CallMethod(this->m_classInstance, method, format);
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
            error = python2Translate(error);
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
    auto gstate = PyGILState_Ensure();

#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->m_classInstance, "set", "(s)", args_toLocal8Bit.data());
#else
    char method[] = "set";
    char format[] = "(s)";
    auto py_retval = PyObject_CallMethod(this->m_classInstance, method, format, args_toLocal8Bit.data());
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
            error = python2Translate(error);
        }
        retval = (successed == Py_True);
        break;
    } while (0);

    Py_XDECREF(py_retval);
    PyGILState_Release(gstate);

    return retval;
}

bool ReinforcementPython::backup(QString &args, QString &error)
{
    auto gstate = PyGILState_Ensure();
#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->m_classInstance, "backup", NULL);
#else
    char method[] = "backup";
    char *format = NULL;
    auto py_retval = PyObject_CallMethod(this->m_classInstance, method, format);
#endif

    bool retval = true;

    do
    {
        if (!this->checkCallResult(py_retval, this->m_className + ".backup", error))
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

bool ReinforcementPython::rollback(const QString &args, QString &error)
{
    auto args_toLocal8Bit = args.toLocal8Bit();
    auto gstate = PyGILState_Ensure();

#if PY_MAJOR_VERSION >= 3
    auto py_retval = PyObject_CallMethod(this->m_classInstance, "rollback", "(s)", args_toLocal8Bit.data());
#else
    char method[] = "rollback";
    char format[] = "(s)";
    auto py_retval = PyObject_CallMethod(this->m_classInstance, method, format, args_toLocal8Bit.data());
#endif

    bool retval = true;
    do
    {
        if (!this->checkCallResult(py_retval, this->m_className + ".rollback", error))
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

    bool isString = false;

#if PY_MAJOR_VERSION < 3
    isString = PyString_Check(pyArg2);
#endif

    if (!PyBool_Check(pyArg1) || (!PyUnicode_Check(pyArg2) && !isString))
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
    auto packageName = PyModule_GetName(this->m_module);

    do
    {
        pyReinforcements = PyObject_GetAttrString(this->m_module, PYTHON_PLUGIN_VAR_REINFORCEMENTS);
        if (!pyReinforcements || !PyTuple_Check(pyReinforcements))
        {
            KLOG_WARNING("Cannot find variable: %s.", PYTHON_PLUGIN_VAR_REINFORCEMENTS);
            break;
        }

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

            KLOG_DEBUG().nospace() << "The " << i << "-th Reinforcement is " << reinforcementName
                                   << ", module name is " << moduleName
                                   << ", class name is " << className;

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

    KLOG_INFO() << "Python plugin" << packageName << "is activated. The plugin contains reinforcements" << m_reinforcements.keys();
}

void PluginPython::deactivate()
{
    clean();
}

void PluginPython::clean()
{
    for (auto iter = this->m_reinforcementsModules.begin(); iter != this->m_reinforcementsModules.end(); ++iter)
    {
        Py_XDECREF(iter.value());
    }
    this->m_reinforcements.clear();
    this->m_reinforcementsModules.clear();
}

void PluginPython::addReinforcement(const QString &packageName,
                                    const QString &moduleName,
                                    const QString &reinforcementName,
                                    const QString &functionPrefix)
{
    // auto module_fullname = fmt::format("{0}.{1}", package_name, module_name);
    auto moduleFullname = QString("%1.%2").arg(packageName, moduleName);

    auto pyModule = this->getReinforcementModule(moduleFullname);
    if (!pyModule)
    {
        pyModule = PyImport_ImportModule(moduleFullname.toLatin1());
        if (!pyModule)
        {
            KLOG_WARNING() << "Failed to load module: " << moduleFullname.toLatin1() << ", error: " << Utils::pyCatchException().toLatin1() << ".";
            return;
        }
        this->m_reinforcementsModules[moduleFullname] = pyModule;
    }

    auto reinforcement = QSharedPointer<ReinforcementPython>(new ReinforcementPython(pyModule, functionPrefix));
    if (this->m_reinforcements.find(reinforcementName) != this->m_reinforcements.end())
    {
        KLOG_WARNING() << "The reinforcement " << reinforcementName.toLatin1() << " is repeated.";
        return;
    }
    else
    {
        this->m_reinforcements[reinforcementName] = reinforcement;
    }
}

}  // namespace BR
}  // namespace KS
