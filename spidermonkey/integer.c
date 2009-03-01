#include "spidermonkey.h"

jsval
py2js_integer(Context* cx, PyObject* obj)
{
    long pyval;
    jsval rval;
    
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
    
    if(INT_FITS_IN_JSVAL(pyval))
    {
        rval = INT_TO_JSVAL(pyval);
    }
    else
    {
        if(!JS_NewNumberValue(cx->cx, pyval, &rval))
        {
            PyErr_SetString(PyExc_ValueError, "Failed to convert number.");
            return JSVAL_VOID;
        }
    }
    
    return rval;
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
