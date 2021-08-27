/**
 * @file          /kiran-ssr-manager/src/daemon/python/python-log.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/daemon/python/python-log.h"
#include "lib/base/base.h"

static PyObject *
log_debug(PyObject *self, PyObject *args)
{
    const char *msg = NULL;
    PyArg_ParseTuple(args, "s", &msg);
    KLOG_DEBUG("%s", msg);
    Py_RETURN_NONE;
}

static PyMethodDef log_methods[] = {
    {"debug", (PyCFunction)log_debug, METH_VARARGS,
     "Print a debug log."},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef logmodule = {
    PyModuleDef_HEAD_INIT,
    "ssr.log",
    NULL,
    -1,
    log_methods};

PyMODINIT_FUNC
PyInit_log(void)
{
    return PyModule_Create(&logmodule);
}