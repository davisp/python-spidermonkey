/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

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
