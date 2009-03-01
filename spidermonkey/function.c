#include "spidermonkey.h"

jsval
py2js_function(Context* cx, PyObject* obj)
{
    return JSVAL_VOID;
}

PyObject*
js2py_function(Context* cx, jsval val)
{
    Py_RETURN_NONE;
}
