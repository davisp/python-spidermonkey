/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

jsval
py2js_integer(Context* cx, PyObject* obj)
{
    long pyval;
    jsval ret = JSVAL_VOID;
    
    if(PyInt_Check(obj))
    {
        pyval = PyInt_AsLong(obj);
        if(PyErr_Occurred()) goto error;
    }
    else
    {
        pyval = PyLong_AsLong(obj);
        if(PyErr_Occurred()) goto error;
    }
    
    if(INT_FITS_IN_JSVAL(pyval))
    {
        ret = INT_TO_JSVAL(pyval);
        goto success;
    }
    
    if(!JS_NewNumberValue(cx->cx, pyval, &ret))
    {
        PyErr_SetString(PyExc_ValueError, "Failed to convert number.");
        goto error;
    }

    goto success;

error:
success:
    return ret;
}

PyObject*
js2py_integer(Context* cx, jsval val)
{
    int32 rval;

    if(!JS_ValueToInt32(cx->cx, val, &rval))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid JS integer value.");
        return NULL;
    }

    return PyInt_FromLong(rval);
}
