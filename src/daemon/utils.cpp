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
}  // namespace Daemon
}  // namespace Kiran
