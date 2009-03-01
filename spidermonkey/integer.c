#include "spidermonkey.h"

jsval
py2js_integer(Context* cx, PyObject* obj)
{
    return JSVAL_VOID;
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
