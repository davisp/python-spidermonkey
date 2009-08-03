
#include <spidermonkey.h>

PyObject*
js2py_function(Context* cx, jsval val, jsval parent)
{
    if(parent == JSVAL_VOID || !JSVAL_IS_OBJECT(parent))
    {
        PyErr_BadInternalCall();
        return NULL;
    }
    
    PyXDR<Function> ret = (Function*) make_object(FunctionType, cx, val);
    if(!ret) return NULL;

    ret->parent = parent;
    if(!JS_AddRoot(cx->cx, &(ret->parent)))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to add GC root.");
        return NULL;
    }

    Function* ptr = ret.get();
    ret.release(); // no decref on success
    return (PyObject*) ptr;
}
