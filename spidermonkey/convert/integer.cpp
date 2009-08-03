/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "convert.h"

jsval
py2js_integer(Context* cx, PyObject* obj)
{
    long pyval;
    
    if(PyInt_Check(obj))
    {
        pyval = PyInt_AsLong(obj);
        if(PyErr_Occurred()) return JSVAL_VOID;
    }
    else
    {
        pyval = PyLong_AsLong(obj);
        if(PyErr_Occurred()) return JSVAL_VOID;
    }

    return long2js_integer(cx, pyval);
}

jsval
long2js_integer(Context* cx, long pyval)
{
    if(INT_FITS_IN_JSVAL(pyval)) return INT_TO_JSVAL(pyval);
    
    jsval ret;
    if(JS_NewNumberValue(cx->cx, pyval, &ret)) return ret;
    
    PyErr_SetString(PyExc_ValueError, "Failed to convert number.");
    return JSVAL_VOID;
}

PyObject*
js2py_integer(Context* cx, jsval val)
{
    int32 ret;
    if(JS_ValueToInt32(cx->cx, val, &ret)) return PyInt_FromLong(ret);

    PyErr_SetString(PyExc_TypeError, "Invalid JS integer value.");
    return NULL;
}
