/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-utils.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "lib/base/base.h"
#include "src/daemon/ssr-utils.h"

namespace Kiran
{
std::string SSRUtils::PyUnicode_AsString(PyObject *unicode)
{
    auto bytes = PyUnicode_AsASCIIString(unicode);
    RETURN_VAL_IF_FALSE(bytes, std::string());
    SCOPE_EXIT({
        Py_XDECREF(bytes);
    });
    auto str = PyBytes_AsString(bytes);
    return POINTER_TO_STRING(str);
}
}  // namespace Kiran
