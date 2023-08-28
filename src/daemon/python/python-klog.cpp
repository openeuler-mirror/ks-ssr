/**
 * @file          /ks-ssr-manager/src/daemon/python/python-klog.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/daemon/python/python-klog.h"
#include "lib/base/base.h"

static PyObject *klog_debug(PyObject *self, PyObject *args)
{
    const char *msg = NULL;
    PyArg_ParseTuple(args, "s", &msg);
    KLOG_DEBUG("%s", msg);
    Py_RETURN_NONE;
}

static PyObject *klog_info(PyObject *self, PyObject *args)
{
    const char *msg = NULL;
    PyArg_ParseTuple(args, "s", &msg);
    KLOG_INFO("%s", msg);
    Py_RETURN_NONE;
}

static PyObject *klog_warning(PyObject *self, PyObject *args)
{
    const char *msg = NULL;
    PyArg_ParseTuple(args, "s", &msg);
    KLOG_WARNING("%s", msg);
    Py_RETURN_NONE;
}

static PyObject *klog_error(PyObject *self, PyObject *args)
{
    const char *msg = NULL;
    PyArg_ParseTuple(args, "s", &msg);
    KLOG_ERROR("%s", msg);
    Py_RETURN_NONE;
}

static PyObject *klog_fatal(PyObject *self, PyObject *args)
{
    const char *msg = NULL;
    PyArg_ParseTuple(args, "s", &msg);
    KLOG_FATAL("%s", msg);
    Py_RETURN_NONE;
}

static PyMethodDef klog_methods[] = {
    {"debug", (PyCFunction)klog_debug, METH_VARARGS, "Print a debug log."},
    {"info", (PyCFunction)klog_info, METH_VARARGS, "Print a info log."},
    {"warning", (PyCFunction)klog_warning, METH_VARARGS, "Print a warning log."},
    {"error", (PyCFunction)klog_error, METH_VARARGS, "Print a error log."},
    {"fatal", (PyCFunction)klog_fatal, METH_VARARGS, "Print a fatal log."},
    {NULL, NULL, 0, NULL}};

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef klogmodule = {
    PyModuleDef_HEAD_INIT,
    "klog",
    NULL,
    -1,
    klog_methods};

PyMODINIT_FUNC PyInit_klog(void)
{
    return PyModule_Create(&klogmodule);
}

#else

void PyInit_klog(void)
{
    Py_InitModule("klog", klog_methods);
}

#endif
