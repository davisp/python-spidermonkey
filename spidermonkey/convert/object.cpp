
#include <spidermonkey.h>

jsval
py2js_object(Context* cx, PyObject* pyobj)
{
    PyObject* hashable = NULL;
    PyObject* attached = NULL;
    JSClass* klass = NULL;
    JSObject* jsobj = NULL;
    jsval pyval;
    jsval ret = JSVAL_VOID;
   
    klass = create_class(cx, pyobj);
    if(klass == NULL) goto error;

    jsobj = JS_NewObject(cx->cx, klass, NULL, NULL);
    if(jsobj == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create JS object.");
        goto error;
    }

    // do the attached = pyobj dance to only DECREF if we get passed INCREF
    attached = pyobj;
    // INCREF for the value stored in JS
    Py_INCREF(attached);
    pyval = PRIVATE_TO_JSVAL(attached);
    if(!JS_SetReservedSlot(cx->cx, jsobj, 0, pyval))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store ref'ed object.");
        goto error;
    }

    hashable = HashCObj_FromVoidPtr(attached);
    if(hashable == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to make hashable pointer.");
        goto error;
    }

    if(Context_add_object(cx, hashable) < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store reference.");
        goto error;
    }

    ret = OBJECT_TO_JSVAL(jsobj);
    goto success;

error:
    Py_XDECREF(attached);
success:
    return ret;
}

PyObject*
js2py_object(Context* cx, jsval val)
{
    return make_object(ObjectType, cx, val);
}