#ifndef PYSM_JSOBJECT_H
#define PYSM_JSOBJECT_H

/*
    This is a representation of a JavaScript
    object in Python land.
*/

#include <Python.h>
#include "structmember.h"

#include "spidermonkey.h"

typedef struct {
    PyObject_HEAD
    Context* cx;
    JSObject* jsobj;
} Object;

extern PyTypeObject _ObjectType;

PyObject* js2py_object(Context* cx, jsval val);

#endif
