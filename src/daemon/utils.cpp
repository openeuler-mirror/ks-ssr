/**
 * @file          /kiran-ssr-manager/src/daemon/utils.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "src/daemon/utils.h"

namespace Kiran
{
namespace Daemon
{
std::string Utils::PyUnicode_AsString(PyObject *unicode)
{
    auto bytes = PyUnicode_AsASCIIString(unicode);
    RETURN_VAL_IF_FALSE(bytes, std::string());
    SCOPE_EXIT({
        Py_XDECREF(bytes);
    });
    auto str = PyBytes_AsString(bytes);
    return POINTER_TO_STRING(str);
}

std::string Utils::catch_exception()
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
    if (PyUnicode_Check(value))
    {
        retval = Utils::PyUnicode_AsString(value);
    }

    if (traceback != NULL)
    {
        retval += "Traceback:";

        auto trace_module_name = PyUnicode_FromString("traceback");
        auto trace_module = PyImport_Import(trace_module_name);

        SCOPE_EXIT({
            Py_XDECREF(trace_module_name);
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
                            retval += Utils::PyUnicode_AsString(PyList_GetItem(error_list, i));
                        }
                    }
                }
            }
        }
    }
    return retval;
}
}  // namespace Daemon
}  // namespace Kiran
