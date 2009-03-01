#include "spidermonkey.h"

jsval
py2js_double(Context* cx, PyObject* obj)
{
    jsval rval;
    double pyval = PyFloat_AsDouble(obj);
    
    if(!JS_NewNumberValue(cx->cx, pyval, &rval))
    {
        PyErr_SetString(PyExc_ValueError, "Failed to convert number.");
        return JSVAL_VOID;
    }

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
