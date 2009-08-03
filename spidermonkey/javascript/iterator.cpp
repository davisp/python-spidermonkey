/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include <spidermonkey.h>

static void finalize(JSContext*, JSObject*);
static JSBool call(JSContext*, JSObject*, uintN, jsval*, jsval*);
JSBool def_next(JSContext*, JSObject*, uintN, jsval*, jsval*);
JSBool seq_next(JSContext*, JSObject*, uintN, jsval*, jsval*);

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
    if(!pycx)
    {
        fprintf(stderr, "*** NO PYTHON CONTEXT ***\n");
        return;
    }

    JSRequest req(jscx);

    Py_DECREF(get_js_slot(jscx, jsobj, 0));
    Py_DECREF(get_js_slot(jscx, jsobj, 1));
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
    JSObject* glbl = JS_GetGlobalObject(jscx);
    jsval exc = JSVAL_VOID;
    
    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");

    PyObject* iter = get_js_slot(jscx, jsobj, 1);
    if(!PyIter_Check(iter))
        return js_error(jscx, "Object is not an iterator.");
    
    PyObject* pyobj = get_js_slot(jscx, jsobj, 0);
    if(!pyobj) return js_error(jscx, "Failed to find iterated object.");

    PyObjectXDR next = PyIter_Next(iter);
    if(!next && PyErr_Occurred())
        return js_error(jscx, "Failed to get iterator's next value.");
    
    // We're done iterating. Throw a StopIteration
    if(!next)
    {
        if(JS_GetProperty(jscx, glbl, "StopIteration", &exc))
        {
            JS_SetPendingException(jscx, exc);
            return JS_FALSE;
        }
        
        return js_error(jscx, "Failed to get StopIteration object.");
    }

    JSBool foreach;
    if(!is_for_each(jscx, jsobj, &foreach))
        return js_error(jscx, "Failed to get iterator flag.");

    if(PyMapping_Check(pyobj) && foreach)
    {
        PyObjectXDR value = PyObject_GetItem(pyobj, next.get());
        if(!value) return js_error(jscx, "Failed to get value in 'for each'");
        *rval = py2js(pycx, value.get());
    }
    else
    {
        *rval = py2js(pycx, next.get());
    }

    if(*rval == JSVAL_VOID)
        return js_error(jscx, "Failed to convert iterator value.");
    
    return JS_TRUE;
}

JSBool
seq_next(JSContext* jscx, JSObject* jsobj, uintN argc, jsval* argv, jsval* rval)
{    
    JSObject* glbl = JS_GetGlobalObject(jscx);
    jsval exc = JSVAL_VOID;

    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");
    
    PyObject* pyobj = get_js_slot(jscx, jsobj, 0);
    if(!pyobj) return js_error(jscx, "Failed to get iterated object.");
    if(!PySequence_Check(pyobj))
        return js_error(jscx, "Object is not a sequence.");

    long maxval = PyObject_Length(pyobj);
    if(maxval < 0) return js_error(jscx, "Failed to get sequence length.");

    PyObject* iter = get_js_slot(jscx, jsobj, 1);
    if(!iter) return js_error(jscx, "Failed to get iteration state.");
    if(!PyInt_Check(iter))
        return js_error(jscx, "Invalid iteration state object.");

    long currval = PyInt_AsLong(iter);
    if(currval == -1 && PyErr_Occurred())
        return js_error(jscx, "Failed to get iteration state value.");

    if(currval + 1 > maxval)
    {
        if(JS_GetProperty(jscx, glbl, "StopIteration", &exc))
        {
            JS_SetPendingException(jscx, exc);
            return JS_FALSE;
        }
        
        return js_error(jscx, "Failed to get StopIteration object.");
    }

    PyObjectXDR next = PyInt_FromLong(currval + 1);
    if(!next) return js_error(jscx, "Failed to create next iterator value.");

    // Swap iterator state.
    if(!JS_SetReservedSlot(jscx, jsobj, 1, PRIVATE_TO_JSVAL(next.get())))
        return js_error(jscx, "Failed to store iterator value.");
    Py_INCREF(next.get());
    Py_DECREF(iter);

    JSBool foreach;
    if(!is_for_each(jscx, jsobj, &foreach))
        return js_error(jscx, "Failed to get iterator flag.");

    if(foreach)
    {
        PyObjectXDR value = PyObject_GetItem(pyobj, iter);
        if(!value) return js_error(jscx, "Failed to get element in 'for each'");
        *rval = py2js(pycx, value.get());
    }
    else
    {
        *rval = py2js(pycx, iter);
    }

    if(*rval == JSVAL_VOID)
        return js_error(jscx, "Failed to convert iterator value.");

    return JS_TRUE;
}

JSBool
new_py_def_iter(Context* cx, PyObject* obj, jsval* rval)
{
    *rval = JSVAL_VOID;

    PyObjectXDR pyiter = PyObject_GetIter(obj);
    if(!pyiter)
    {
        if(PyErr_GivenExceptionMatches(PyErr_Occurred(), PyExc_TypeError))
        {
            PyErr_Clear();
            return JS_TRUE;
        }
        
        return JS_FALSE;
    }

    JSObject* jsiter = JS_NewObject(cx->cx, &js_iter_class, NULL, NULL);
    if(jsiter == NULL) return js_error(cx->cx, "Failed to create iterator.");

    if(!JS_DefineFunctions(cx->cx, jsiter, js_def_iter_functions))
        return js_error(cx->cx, "Failed to define iterator functions.");

    PyObjectXDR attached = obj;
    Py_INCREF(attached.get());

    jsval jsv = PRIVATE_TO_JSVAL(attached.get());
    if(!JS_SetReservedSlot(cx->cx, jsiter, 0, jsv))
        return js_error(cx->cx, "Failed to store iterated object.");
    
    jsv = PRIVATE_TO_JSVAL(pyiter.get());
    if(!JS_SetReservedSlot(cx->cx, jsiter, 1, jsv))
        return js_error(cx->cx, "Failed to store iterator object.");

    if(!JS_SetReservedSlot(cx->cx, jsiter, 2, JSVAL_FALSE))
        return js_error(cx->cx, "Failed to store iterator flag.");

    Py_INCREF(cx);
    *rval = OBJECT_TO_JSVAL(jsiter);

    // Keep attached alive on success.
    attached.release();
    return JS_TRUE;
}

JSBool
new_py_seq_iter(Context* cx, PyObject* obj, jsval* rval)
{
    *rval = JSVAL_VOID;

    // Our counting state
    PyObjectXDR pyiter = PyInt_FromLong(0);
    if(!pyiter) return js_error(cx->cx, "Failed to create iterator state.");

    JSObject* jsiter = JS_NewObject(cx->cx, &js_iter_class, NULL, NULL);
    if(jsiter == NULL) return js_error(cx->cx, "Failed to create iterator.");

    if(!JS_DefineFunctions(cx->cx, jsiter, js_seq_iter_functions))
        return js_error(cx->cx, "Failed to define iterator functions.");

    PyObjectXDR attached = obj;
    Py_INCREF(attached.get());

    jsval jsv = PRIVATE_TO_JSVAL(attached.get());
    if(!JS_SetReservedSlot(cx->cx, jsiter, 0, jsv))
        return js_error(cx->cx, "Failed to store iterated object.");
    
    jsv = PRIVATE_TO_JSVAL(pyiter.get());
    if(!JS_SetReservedSlot(cx->cx, jsiter, 1, jsv))
        return js_error(cx->cx, "Failed to store iterator object.");

    if(!JS_SetReservedSlot(cx->cx, jsiter, 2, JSVAL_FALSE))
        return js_error(cx->cx, "Failed to store iterator flag.");

    Py_INCREF(cx);
    *rval = OBJECT_TO_JSVAL(jsiter);
    
    // Keep attached alive on success.
    return JS_TRUE;
}

JSBool
new_py_iter(Context* cx, PyObject* obj, jsval* rval)
{
    if(PySequence_Check(obj))
        return new_py_seq_iter(cx, obj, rval);

    return new_py_def_iter(cx, obj, rval);
}
