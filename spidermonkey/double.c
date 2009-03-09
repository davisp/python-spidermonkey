/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

jsval
py2js_double(Context* cx, PyObject* obj)
{
    jsval rval = JSVAL_VOID;
    double pyval = PyFloat_AsDouble(obj);
    if(PyErr_Occurred()) goto error;

    if(!JS_NewNumberValue(cx->cx, pyval, &rval))
    {
        PyErr_SetString(PyExc_ValueError, "Failed to convert number.");
        goto error;
    }

    goto success;

error:
success:
    return rval;
}

PyObject*
js2py_double(Context* cx, jsval val)
{
    double rval;

    if(!JS_ValueToNumber(cx->cx, val, &rval))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid JS number value.");
        return NULL;
    }

    return PyFloat_FromDouble(rval);
}
