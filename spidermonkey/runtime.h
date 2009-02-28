#ifndef PYSM_RUNTIME
#define PYSM_RUNTIME

#include <Python.h>
#include "structmember.h"

#include "libjs/jsapi.h"

typedef struct {
    PyObject_HEAD
    JSRuntime* rt;
} Runtime;

extern PyTypeObject _RuntimeType;

#endif
