#ifndef PYSM_CONTEXT
#define PYSM_CONTEXT

#include <Python.h>
#include "structmember.h"

#include "spidermonkey.h"

typedef struct {
    PyObject_HEAD
    Runtime* rt;
    JSContext* cx;
    JSObject* root;
} Context;

extern PyTypeObject _ContextType;

#endif
