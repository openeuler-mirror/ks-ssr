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

#include "utils.h"

namespace KS
{
namespace BRDaemon
{
PyThreadState *Utils::main_thread_state_ = NULL;

QString Utils::pyobjectAsString(PyObject *pyobject)
{
    QString retval;

    do
    {
        if (PyUnicode_Check(pyobject))
        {
            auto bytes = PyUnicode_AsASCIIString(pyobject);
            BREAK_IF_FALSE(bytes);
            auto str = PyBytes_AsString(bytes);
            retval = POINTER_TO_STRING(str);
            Py_XDECREF(bytes);
        }
#if PY_MAJOR_VERSION < 3
        else if (PyString_Check(pyobject))
        {
            auto str = PyString_AsString(pyobject);
            retval = POINTER_TO_STRING(str);
        }
#endif
        else if (PyExceptionInstance_Check(pyobject))
        {
            auto args = PyObject_GetAttrString(pyobject, "args");
            if (args && PyTuple_Check(args) && PyTuple_GET_SIZE(args) > 0)
            {
                retval = Utils::pyobjectAsString(PyTuple_GetItem(args, 0));
            }
            Py_XDECREF(args);
        }
    } while (0);

    return retval;
}

QString Utils::pyCatchException()
{
    RETURN_VAL_IF_TRUE(PyErr_Occurred() == NULL, QString());

    PyObject *type = NULL;
    PyObject *value = NULL;
    PyObject *traceback = NULL;
    PyObject *trace_module = NULL;
    QString retval;

    do
    {
        PyErr_Fetch(&type, &value, &traceback);
        BREAK_IF_TRUE(value == NULL);

        PyErr_NormalizeException(&type, &value, 0);
        retval = Utils::pyobjectAsString(value);

        if (traceback != NULL)
        {
            retval += "Traceback:";

            trace_module = PyImport_ImportModule("traceback");

            if (trace_module != NULL)
            {
                auto trace_module_dict = PyModule_GetDict(trace_module);
                if (trace_module_dict != NULL)
                {
                    auto format_func = PyDict_GetItemString(trace_module_dict, "format_exception");
                    if (format_func != NULL)
                    {
                        auto error_list = PyObject_CallFunctionObjArgs(format_func, type, value, traceback, NULL);
                        if (error_list != NULL)
                        {
                            auto list_size = PyList_Size(error_list);
                            for (int i = 0; i < list_size; ++i)
                            {
                                retval += Utils::pyobjectAsString(PyList_GetItem(error_list, i));
                            }
                        }
                    }
                }
            }
        }
    } while (0);

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
    Py_XDECREF(trace_module);

    return retval;
}

void Utils::pyGiLock()
{
    if (Utils::main_thread_state_ != NULL)
    {
        PyEval_RestoreThread(Utils::main_thread_state_);
        Utils::main_thread_state_ = NULL;
    }
    else
    {
        KLOG_WARNING("The main thread has held GIL already.");
    }
}

void Utils::pyGiUnlock()
{
    if (Utils::main_thread_state_ == NULL)
    {
        Utils::main_thread_state_ = PyEval_SaveThread();
    }
    else
    {
        KLOG_WARNING("The main thread has released GIL already.");
    }
}
}  // namespace BRDaemon
}  // namespace KS
