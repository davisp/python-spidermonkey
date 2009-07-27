
#include "convert.h"

PyObject*
js2py_function(Context* cx, jsval val, jsval parent)
{
    Function* ret = NULL;

    if(parent == JSVAL_VOID || !JSVAL_IS_OBJECT(parent))
    {
        PyErr_BadInternalCall();
        goto error;
    }
    
    ret = (Function*) make_object(FunctionType, cx, val);
    if(ret == NULL) goto error;

    ret->parent = parent;
    if(!JS_AddRoot(cx->cx, &(ret->parent)))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to add GC root.");
        goto error;
    }

    goto success;

error:
    Py_XDECREF((PyObject*)ret);
    ret = NULL; // In case of AddRoot error.
success:
    return (PyObject*) ret;
}
