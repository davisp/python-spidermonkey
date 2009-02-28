#ifndef PYSM_CLASS_H
#define PYSM_CLASS_H

#include <Python.h>
#include "structmember.h"

#include "spidermonkey.h"

typedef struct {
    PyObject_HEAD
    PyObject* cx;
    PyObject* pyclass;
    JSClass* jsclass;
    JSObject* prototype;
} Class;

extern PyTypeObject _ClassType;

#endif
