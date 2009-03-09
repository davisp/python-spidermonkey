/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_JSFUNCTION_H
#define PYSM_JSFUNCTION_H

/*
    This is a representation of a JavaScript
    Function in Python land.
*/

typedef struct {
    Object obj;
    jsval parent;
} Function;

extern PyTypeObject _FunctionType;

PyObject* js2py_function(Context* cx, jsval val, jsval parent);

#endif
