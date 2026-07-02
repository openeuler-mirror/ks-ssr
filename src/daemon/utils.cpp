/**
 * @file          /ks-ssr-manager/src/daemon/utils.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "src/daemon/utils.h"

namespace KS
{
namespace Daemon
{
PyThreadState *Utils::main_thread_state_ = NULL;

std::string Utils::pyobject_as_string(PyObject *pyobject)
{
    if (PyUnicode_Check(pyobject))
    {
        auto bytes = PyUnicode_AsASCIIString(pyobject);
        RETURN_VAL_IF_FALSE(bytes, std::string());
        SCOPE_EXIT({
            Py_XDECREF(bytes);
        });
        auto str = PyBytes_AsString(bytes);
        return POINTER_TO_STRING(str);
    }
#if PY_MAJOR_VERSION < 3
    else if (PyString_Check(pyobject))
    {
        auto str = PyString_AsString(pyobject);
        return POINTER_TO_STRING(str);
    }
#endif
    else if (PyExceptionInstance_Check(pyobject))
    {
        auto args = PyObject_GetAttrString(pyobject, "args");
        SCOPE_EXIT({
            Py_XDECREF(args);
        });
        if (args && PyTuple_Check(args) && PyTuple_GET_SIZE(args) > 0)
        {
            return Utils::pyobject_as_string(PyTuple_GetItem(args, 0));
        }
    }
    return std::string();
}

std::string Utils::py_catch_exception()
{
    RETURN_VAL_IF_TRUE(PyErr_Occurred() == NULL, std::string());

    PyObject *type = NULL;
    PyObject *value = NULL;
    PyObject *traceback = NULL;
    std::string retval;

    SCOPE_EXIT({
        Py_XDECREF(type);
        Py_XDECREF(value);
        Py_XDECREF(traceback);
    });

    PyErr_Fetch(&type, &value, &traceback);
    RETURN_VAL_IF_TRUE(value == NULL, std::string());

    PyErr_NormalizeException(&type, &value, 0);
    retval = Utils::pyobject_as_string(value);

    if (traceback != NULL)
    {
        retval += "Traceback:";

        auto trace_module = PyImport_ImportModule("traceback");

        SCOPE_EXIT({
            Py_XDECREF(trace_module);
        });

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
                            retval += Utils::pyobject_as_string(PyList_GetItem(error_list, i));
                        }
                    }
                }
            }
        }
    }
    return retval;
}

void Utils::py_gi_lock()
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

void Utils::py_gi_unlock()
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
}  // namespace Daemon
}  // namespace KS
