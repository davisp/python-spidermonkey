/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_RUNTIME
#define PYSM_RUNTIME

#include <Python.h>
#include "structmember.h"

#include <jsapi.h>

typedef struct {
    PyObject_HEAD
    JSRuntime* rt;
} Runtime;

extern PyTypeObject _RuntimeType;

#endif
