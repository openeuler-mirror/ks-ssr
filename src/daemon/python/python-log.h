/**
 * @file          /kiran-ssr-manager/src/daemon/python/python-log.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if PY_MAJOR_VERSION >= 3
    PyMODINIT_FUNC PyInit_log(void);
#else
void PyInit_log(void);
#endif

#ifdef __cplusplus
}
#endif
