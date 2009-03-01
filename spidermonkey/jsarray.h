#ifndef PYSM_JSARRAY_H
#define PYSM_JSARRAY_H

/*
    This is a representation of a JavaScript
    Array in Python land.
*/

#include <Python.h>
#include "structmember.h"

#include "spidermonkey.h"

extern PyTypeObject _ArrayType;

PyObject* js2py_array(Context* cx, jsval val);

#endif
