
#include <spidermonkey.h>

jsval
py2js_object(Context* cx, PyObject* pyobj)
{
    // PyObject* hashable = NULL;
    // PyObject* attached = NULL;
    // JSClass* klass = NULL;
    // JSObject* jsobj = NULL;
    // jsval pyval;
    // jsval ret = JSVAL_VOID;
   
    JSClass* klass = create_class(cx, pyobj);
    if(klass == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create JS class.");
        return JSVAL_VOID;
    }

    JSObject* jsobj = JS_NewObject(cx->cx, klass, NULL, NULL);
    if(jsobj == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create JS object.");
        return JSVAL_VOID;
    }

    PyObjectXDR attached = pyobj;
    Py_INCREF(attached.get());
    jsval pyval = PRIVATE_TO_JSVAL(attached.get());
    if(!JS_SetReservedSlot(cx->cx, jsobj, 0, pyval))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store ref'ed object.");
        return JSVAL_VOID;
    }

    PyObject* hashable = HashCObj_FromVoidPtr(attached.get());
    if(!hashable)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to make hashable pointer.");
        return JSVAL_VOID;
    }

    if(Context_add_object(cx, hashable) < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store reference.");
        return JSVAL_VOID;
    }

    attached.release(); // No decref on success
    return OBJECT_TO_JSVAL(jsobj);
}

PyObject*
js2py_object(Context* cx, jsval val)
{
    return make_object(ObjectType, cx, val);
}