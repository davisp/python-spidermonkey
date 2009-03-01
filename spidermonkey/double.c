#include "spidermonkey.h"

jsval
py2js_double(Context* cx, PyObject* obj)
{
    return JSVAL_VOID;
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
