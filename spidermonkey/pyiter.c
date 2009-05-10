/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

PyObject*
get_js_slot(JSContext* cx, JSObject* obj, int slot)
{
    jsval priv;

    if(!JS_GetReservedSlot(cx, obj, slot, &priv))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get slot data.");
        return NULL;
    }

    return (PyObject*) JSVAL_TO_PRIVATE(priv);
}

void
finalize(JSContext* jscx, JSObject* jsobj)
{
    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    PyObject* pyobj = NULL;
    PyObject* pyiter = NULL;

    JS_BeginRequest(jscx);

    if(pycx == NULL)
    {
        fprintf(stderr, "*** NO PYTHON CONTEXT ***\n");
        return;
    }

    pyobj = get_js_slot(jscx, jsobj, 0);
    Py_DECREF(pyobj);

    pyiter = get_js_slot(jscx, jsobj, 1);
    Py_DECREF(pyiter);

    JS_EndRequest(jscx);

    Py_DECREF(pycx);
}

JSBool
call(JSContext* jscx, JSObject* jsobj, uintN argc, jsval* argv, jsval* rval)
{
    jsval objval = argv[-2];
    JSObject* obj = JSVAL_TO_OBJECT(objval);

    if(argc >= 1 && JSVAL_IS_BOOLEAN(argv[0]) && !JSVAL_TO_BOOLEAN(argv[0]))
    {
        if(!JS_SetReservedSlot(jscx, obj, 2, JSVAL_TRUE))
        {
            JS_ReportError(jscx, "Failed to reset iterator flag.");
            return JS_FALSE;
        }
    }

    *rval = argv[-2];
    return JS_TRUE;
}

JSBool
is_for_each(JSContext* cx, JSObject* obj, JSBool* rval)
{
    jsval slot;
    if(!JS_GetReservedSlot(cx, obj, 2, &slot))
    {
        return JS_FALSE;
    }

    if(!JSVAL_IS_BOOLEAN(slot)) return JS_FALSE;
    *rval = JSVAL_TO_BOOLEAN(slot);
    return JS_TRUE;
}

JSBool
def_next(JSContext* jscx, JSObject* jsobj, uintN argc, jsval* argv, jsval* rval)
{
    Context* pycx = NULL;
    PyObject* pyobj = NULL;
    PyObject* iter = NULL;
    PyObject* next = NULL;
    PyObject* value = NULL;
    JSBool ret = JS_FALSE;
    JSBool foreach = JS_FALSE;

    // For StopIteration throw
    JSObject* glbl = JS_GetGlobalObject(jscx);
    jsval exc = JSVAL_VOID;

    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get JS Context.");
        goto done;
    }

    iter = get_js_slot(jscx, jsobj, 1);
    if(!PyIter_Check(iter))
    {
        JS_ReportError(jscx, "Object is not an iterator.");
        goto done;
    }

    pyobj = get_js_slot(jscx, jsobj, 0);
    if(pyobj == NULL)
    {
        JS_ReportError(jscx, "Failed to find iterated object.");
        goto done;
    }

    next = PyIter_Next(iter);
    if(next == NULL && PyErr_Occurred())
    {
        goto done;
    }
    else if(next == NULL)
    {
        if(JS_GetProperty(jscx, glbl, "StopIteration", &exc))
        {
            JS_SetPendingException(jscx, exc);
        }
        else
        {
            JS_ReportError(jscx, "Failed to get StopIteration object.");
        }
        goto done;
    }

    if(!is_for_each(jscx, jsobj, &foreach))
    {
        JS_ReportError(jscx, "Failed to get iterator flag.");
        goto done;
    }

    if(PyMapping_Check(pyobj) && foreach)
    {
        value = PyObject_GetItem(pyobj, next);
        if(value == NULL)
        {
            JS_ReportError(jscx, "Failed to get value in 'for each'");
            goto done;
        }
        *rval = py2js(pycx, value);
    }
    else
    {
        *rval = py2js(pycx, next);
    }

    if(*rval != JSVAL_VOID) ret = JS_TRUE;

done:
    Py_XDECREF(next);
    Py_XDECREF(value);
    return ret;
}

JSBool
seq_next(JSContext* jscx, JSObject* jsobj, uintN argc, jsval* argv, jsval* rval)
{
    Context* pycx = NULL;
    PyObject* pyobj = NULL;
    PyObject* iter = NULL;
    PyObject* next = NULL;
    PyObject* value = NULL;
    JSBool ret = JS_FALSE;
    JSBool foreach = JS_FALSE;
    long maxval = -1;
    long currval = -1;
    
    // For StopIteration throw
    JSObject* glbl = JS_GetGlobalObject(jscx);
    jsval exc = JSVAL_VOID;

    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get JS Context.");
        goto done;
    }

    pyobj = get_js_slot(jscx, jsobj, 0);
    if(!PySequence_Check(pyobj))
    {
        JS_ReportError(jscx, "Object is not a sequence.");
        goto done;
    }

    maxval = PyObject_Length(pyobj);
    if(maxval < 0) goto done;

    iter = get_js_slot(jscx, jsobj, 1);
    if(!PyInt_Check(iter))
    {
        JS_ReportError(jscx, "Object is not an integer.");
        goto done;
    }

    currval = PyInt_AsLong(iter);
    if(currval == -1 && PyErr_Occurred())
    {
        goto done;
    }
    
    if(currval + 1 > maxval)
    {
        if(JS_GetProperty(jscx, glbl, "StopIteration", &exc))
        {
            JS_SetPendingException(jscx, exc);
        }
        else
        {
            JS_ReportError(jscx, "Failed to get StopIteration object.");
        }
        goto done;
    }

    next = PyInt_FromLong(currval + 1);
    if(next == NULL) goto done;

    if(!JS_SetReservedSlot(jscx, jsobj, 1, PRIVATE_TO_JSVAL(next)))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store base object.");
        goto done;
    }

    if(!is_for_each(jscx, jsobj, &foreach))
    {
        JS_ReportError(jscx, "Failed to get iterator flag.");
        goto done;
    }

    if(foreach)
    {
        value = PyObject_GetItem(pyobj, iter);
        if(value == NULL)
        {
            JS_ReportError(jscx, "Failed to get array element in 'for each'");
            goto done;
        }
        *rval = py2js(pycx, value);
    }
    else
    {
        *rval = py2js(pycx, iter);
    }

    next = iter;
    if(*rval != JSVAL_VOID) ret = JS_TRUE;

done:
    Py_XDECREF(next);
    Py_XDECREF(value);
    return ret;
}

static JSClass
js_iter_class = {
    "PyJSIteratorClass",
    JSCLASS_HAS_RESERVED_SLOTS(3),
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    finalize,
    NULL, // get object ops
    NULL, // check access
    call,
    NULL, // constructor
    NULL, // xdr object
    NULL, // has instance
    NULL, // mark
    NULL  // reserved slots
};

static JSFunctionSpec js_def_iter_functions[] = {
    {"next", def_next, 0, 0, 0},
    {0, 0, 0, 0, 0}
};

static JSFunctionSpec js_seq_iter_functions[] = {
    {"next", seq_next, 0, 0, 0},
    {0, 0, 0, 0, 0}
};

JSBool
new_py_def_iter(Context* cx, PyObject* obj, jsval* rval)
{
    PyObject* pyiter = NULL;
    PyObject* attached = NULL;
    JSObject* jsiter = NULL;
    jsval jsv = JSVAL_VOID;
    JSBool ret = JS_FALSE;

    // Initialize the return value
    *rval = JSVAL_VOID;

    pyiter = PyObject_GetIter(obj);
    if(pyiter == NULL)
    {
        if(PyErr_GivenExceptionMatches(PyErr_Occurred(), PyExc_TypeError))
        {
            PyErr_Clear();
            ret = JS_TRUE;
            goto success;
        }
        else
        {
            goto error;
        }
    }

    jsiter = JS_NewObject(cx->cx, &js_iter_class, NULL, NULL);
    if(jsiter == NULL) goto error;

    if(!JS_DefineFunctions(cx->cx, jsiter, js_def_iter_functions))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to define iter funcions.");
        goto error;
    }

    attached = obj;
    Py_INCREF(attached);
    jsv = PRIVATE_TO_JSVAL(attached);
    if(!JS_SetReservedSlot(cx->cx, jsiter, 0, jsv))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store base object.");
        goto error;
    }

    jsv = PRIVATE_TO_JSVAL(pyiter);
    if(!JS_SetReservedSlot(cx->cx, jsiter, 1, jsv))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store iter object.");
        goto error;
    }

    if(!JS_SetReservedSlot(cx->cx, jsiter, 2, JSVAL_FALSE))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store iterator flag.");
        goto error;
    }

    Py_INCREF(cx);
    *rval = OBJECT_TO_JSVAL(jsiter);
    ret = JS_TRUE;
    goto success;

error:
    Py_XDECREF(pyiter);
    Py_XDECREF(attached);
success:
    return ret;
}

JSBool
new_py_seq_iter(Context* cx, PyObject* obj, jsval* rval)
{
    PyObject* pyiter = NULL;
    PyObject* attached = NULL;
    JSObject* jsiter = NULL;
    jsval jsv = JSVAL_VOID;
    JSBool ret = JS_FALSE;

    // Initialize the return value
    *rval = JSVAL_VOID;

    // Our counting state
    pyiter = PyInt_FromLong(0);
    if(pyiter == NULL) goto error;

    jsiter = JS_NewObject(cx->cx, &js_iter_class, NULL, NULL);
    if(jsiter == NULL) goto error;

    if(!JS_DefineFunctions(cx->cx, jsiter, js_seq_iter_functions))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to define iter funcions.");
        goto error;
    }

    attached = obj;
    Py_INCREF(attached);
    jsv = PRIVATE_TO_JSVAL(attached);
    if(!JS_SetReservedSlot(cx->cx, jsiter, 0, jsv))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store base object.");
        goto error;
    }

    jsv = PRIVATE_TO_JSVAL(pyiter);
    if(!JS_SetReservedSlot(cx->cx, jsiter, 1, jsv))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store iter object.");
        goto error;
    }

    if(!JS_SetReservedSlot(cx->cx, jsiter, 2, JSVAL_FALSE))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to store iterator flag.");
        goto error;
    }

    Py_INCREF(cx);
    *rval = OBJECT_TO_JSVAL(jsiter);
    ret = JS_TRUE;
    goto success;

error:
    Py_XDECREF(pyiter);
    Py_XDECREF(attached);
success:
    return ret;
}

JSBool
new_py_iter(Context* cx, PyObject* obj, jsval* rval)
{
    if(PySequence_Check(obj))
    {
        return new_py_seq_iter(cx, obj, rval);
    }
    return new_py_def_iter(cx, obj, rval);
}
