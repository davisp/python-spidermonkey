/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

#include <time.h> // After spidermonkey.h so after Python.h

#include <jsobj.h>
#include <jscntxt.h>

JSBool
add_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    JSObject* obj = NULL;

    if(!JSVAL_IS_OBJECT(*rval)) return JS_TRUE;

    obj = JSVAL_TO_OBJECT(*rval);
    if(JS_ObjectIsFunction(jscx, obj)) return set_prop(jscx, jsobj, key, rval);
    return JS_TRUE;
}

JSBool
del_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    Context* pycx = NULL;
    PyObject* pykey = NULL;
    PyObject* pyval = NULL;
    JSBool ret = JS_FALSE;

    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get Python context.");
        goto done;
    }

    // Bail if there's no registered global handler.
    if(pycx->global == NULL)
    {
        ret = JS_TRUE;
        goto done;
    }

    // Bail if the global doesn't have a __delitem__
    if(!PyObject_HasAttrString(pycx->global, "__delitem__"))
    {
        ret = JS_TRUE;
        goto done;
    }

    pykey = js2py(pycx, key);
    if(pykey == NULL) goto done;

    if(PyObject_DelItem(pycx->global, pykey) < 0) goto done;

    ret = JS_TRUE;

done:
    Py_XDECREF(pykey);
    Py_XDECREF(pyval);
    return ret;
}

JSBool
get_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    Context* pycx = NULL;
    PyObject* pykey = NULL;
    PyObject* pyval = NULL;
    JSBool ret = JS_FALSE;

    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get Python context.");
        goto done;
    }

    // Bail if there's no registered global handler.
    if(pycx->global == NULL)
    {
        ret = JS_TRUE;
        goto done;
    }

    pykey = js2py(pycx, key);
    if(pykey == NULL) goto done;

    pyval = PyObject_GetItem(pycx->global, pykey);
    if(pyval == NULL)
    {
        if(PyErr_GivenExceptionMatches(PyErr_Occurred(), PyExc_KeyError))
        {
            PyErr_Clear();
            ret = JS_TRUE;
        }
        goto done;
    }

    *rval = py2js(pycx, pyval);
    if(*rval == JSVAL_VOID) goto done;
    ret = JS_TRUE;

done:
    Py_XDECREF(pykey);
    Py_XDECREF(pyval);
    return ret;
}

JSBool
set_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    Context* pycx = NULL;
    PyObject* pykey = NULL;
    PyObject* pyval = NULL;
    JSBool ret = JS_FALSE;

    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get Python context.");
        goto done;
    }

    // Bail if there's no registered global handler.
    if(pycx->global == NULL)
    {
        ret = JS_TRUE;
        goto done;
    }

    pykey = js2py(pycx, key);
    if(pykey == NULL) goto done;

    pyval = js2py(pycx, *rval);
    if(pyval == NULL) goto done;

    if(PyObject_SetItem(pycx->global, pykey, pyval) < 0) goto done;

    ret = JS_TRUE;

done:
    Py_XDECREF(pykey);
    Py_XDECREF(pyval);
    return ret;
}

JSBool
resolve(JSContext* jscx, JSObject* jsobj, jsval key)
{
    Context* pycx = NULL;
    PyObject* pykey = NULL;
    jsid pid;
    JSBool ret = JS_FALSE;

    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get Python context.");
        goto done;
    }

    // Bail if there's no registered global handler.
    if(pycx->global == NULL)
    {
        ret = JS_TRUE;
        goto done;
    }

    pykey = js2py(pycx, key);
    if(pykey == NULL) goto done;

    if(!PyMapping_HasKey(pycx->global, pykey))
    {
        ret = JS_TRUE;
        goto done;
    }

    if(!JS_ValueToId(jscx, key, &pid))
    {
        JS_ReportError(jscx, "Failed to convert property id.");
        goto done;
    }

    if(!js_DefineProperty(jscx, pycx->root, pid, JSVAL_VOID, NULL, NULL,
                            JSPROP_SHARED, NULL))
    {
        JS_ReportError(jscx, "Failed to define property.");
        goto done;
    }

    ret = JS_TRUE;

done:
    Py_XDECREF(pykey);
    return ret;
}

static JSClass
js_global_class = {
    "JSGlobalObjectClass",
    JSCLASS_GLOBAL_FLAGS,
    add_prop,
    del_prop,
    get_prop,
    set_prop,
    JS_EnumerateStub,
    resolve,
    JS_ConvertStub,
    JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

#define MAX(a, b) ((a) > (b) ? (a) : (b))
JSBool
branch_cb(JSContext* jscx, JSScript* script)
{
    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    time_t now = time(NULL);

    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to find Python context.");
        return JS_FALSE;
    }

    // Get out quick if we don't have any quotas.
    if(pycx->max_time == 0 && pycx->max_heap == 0)
    {
        return JS_TRUE;
    }

    // Only check occasionally for resource usage.
    pycx->branch_count++;
    if((pycx->branch_count > 0x3FFF) != 1)
    {
        return JS_TRUE;
    }

    pycx->branch_count = 0;

    if(pycx->max_heap > 0 && jscx->runtime->gcBytes > pycx->max_heap)
    {
        // First see if garbage collection gets under the threshold.
        JS_GC(jscx);
        if(jscx->runtime->gcBytes > pycx->max_heap)
        {
            PyErr_NoMemory();
            return JS_FALSE;
        }
    }

    if(
        pycx->max_time > 0
        && pycx->start_time > 0
        && pycx->max_time < now - pycx->start_time
    )
    {
        PyErr_SetNone(PyExc_SystemError);
        return JS_FALSE;
    }

    return JS_TRUE;
}

PyObject*
Context_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    Context* self = NULL;
    Runtime* runtime = NULL;
    PyObject* global = NULL;

    if(!PyArg_ParseTuple(
        args,
        "O!|O",
        RuntimeType, &runtime,
        &global
    )) goto error;

    if(global != NULL && !PyMapping_Check(global))
    {
        PyErr_SetString(PyExc_TypeError,
                            "Global handler must provide item access.");
        goto error;
    }

    self = (Context*) type->tp_alloc(type, 0);
    if(self == NULL) goto error;

    // Tracking what classes we've installed in
    // the context.
    self->classes = (PyDictObject*) PyDict_New();
    if(self->classes == NULL) goto error;

    self->objects = (PySetObject*) PySet_New(NULL);
    if(self->objects == NULL) goto error;

    self->cx = JS_NewContext(runtime->rt, 8192);
    if(self->cx == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create JSContext.");
        goto error;
    }

    JS_BeginRequest(self->cx);

    /*
     *  Notice that we don't add a ref to the Python context for
     *  the copy stored on the JSContext*. I'm pretty sure this
     *  would cause a cyclic dependancy that would prevent
     *  garbage collection from happening on either side of the
     *  bridge.
     *
     *  To make sure that the context stays alive we'll add a
     *  reference to the Context* anytime we wrap a Python
     *  object for use in JS.
     *
     */
    JS_SetContextPrivate(self->cx, self);

    // Setup the root of the property lookup doodad.
    self->root = JS_NewObject(self->cx, &js_global_class, NULL, NULL);
    if(self->root == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Error creating root object.");
        goto error;
    }

    if(!JS_InitStandardClasses(self->cx, self->root))
    {
        PyErr_SetString(PyExc_RuntimeError, "Error initializing JS VM.");
        goto error;
    }

    // Don't setup the global handler until after the standard classes
    // have been initialized.
    // XXX: Does anyone know if finalize is called if new fails?
    if(global != NULL) Py_INCREF(global);
    self->global = global;

    // Setup counters for resource limits
    self->branch_count = 0;
    self->max_time = 0;
    self->start_time = 0;
    self->max_heap = 0;

    JS_SetBranchCallback(self->cx, branch_cb);
    JS_SetErrorReporter(self->cx, report_error_cb);
    
    Py_INCREF(runtime);
    self->rt = runtime;

    goto success;

error:
    if(self != NULL && self->cx != NULL) JS_EndRequest(self->cx);
    Py_XDECREF(self);
    self = NULL;

success:
    if(self != NULL && self->cx != NULL) JS_EndRequest(self->cx);
    return (PyObject*) self;
}

int
Context_init(Context* self, PyObject* args, PyObject* kwargs)
{
    return 0;
}

void
Context_dealloc(Context* self)
{
    if(self->cx != NULL)
    {
        JS_DestroyContext(self->cx);
    }

    Py_XDECREF(self->global);
    Py_XDECREF(self->objects);
    Py_XDECREF(self->classes);
    Py_XDECREF(self->rt);
}

PyObject*
Context_add_global(Context* self, PyObject* args, PyObject* kwargs)
{
    PyObject* pykey = NULL;
    PyObject* pyval = NULL;
    jsval jsk;
    jsid kid;
    jsval jsv;

    JS_BeginRequest(self->cx);

    if(!PyArg_ParseTuple(args, "OO", &pykey, &pyval)) goto error;

    jsk = py2js(self, pykey);
    if(jsk == JSVAL_VOID) goto error;

    if(!JS_ValueToId(self->cx, jsk, &kid))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to create key id.");
        goto error;
    }

    jsv = py2js(self, pyval);
    if(jsv == JSVAL_VOID) goto error;

    if(!js_SetProperty(self->cx, self->root, kid, &jsv))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to set global property.");
        goto error;
    }

    goto success;

error:
success:
    JS_EndRequest(self->cx);
    Py_RETURN_NONE;
}

PyObject*
Context_rem_global(Context* self, PyObject* args, PyObject* kwargs)
{
    PyObject* pykey = NULL;
    PyObject* ret = NULL;
    jsval jsk;
    jsid kid;
    jsval jsv;

    JS_BeginRequest(self->cx);

    if(!PyArg_ParseTuple(args, "O", &pykey)) goto error;

    jsk = py2js(self, pykey);
    if(jsk == JSVAL_VOID) goto error;

    if(!JS_ValueToId(self->cx, jsk, &kid))
    {
        PyErr_SetString(JSError, "Failed to create key id.");
    }

    if(!js_GetProperty(self->cx, self->root, kid, &jsv))
    {
        PyErr_SetString(JSError, "Failed to get global property.");
        goto error;
    }
    
    ret = js2py(self, jsv);
    if(ret == NULL) goto error;
    
    if(!js_DeleteProperty(self->cx, self->root, kid, &jsv))
    {
        PyErr_SetString(JSError, "Failed to remove global property.");
        goto error;
    }

    JS_MaybeGC(self->cx);

    goto success;

error:
success:
    JS_EndRequest(self->cx);
    return ret;
}

PyObject*
Context_execute(Context* self, PyObject* args, PyObject* kwargs)
{
    PyObject* obj = NULL;
    PyObject* ret = NULL;
    JSContext* cx = NULL;
    JSObject* root = NULL;
    JSString* script = NULL;
    jschar* schars = NULL;
    JSBool started_counter = JS_FALSE;
    size_t slen;
    jsval rval;

    JS_BeginRequest(self->cx);
    if(!PyArg_ParseTuple(args, "O", &obj)) goto error;
    
    script = py2js_string_obj(self, obj);
    if(script == NULL) goto error;

    schars = JS_GetStringChars(script);
    slen = JS_GetStringLength(script);
    
    cx = self->cx;
    root = self->root;

    // Mark us for time consumption
    if(self->start_time == 0)
    {
        started_counter = JS_TRUE;
        self->start_time = time(NULL);
    }

    if(!JS_EvaluateUCScript(cx, root, schars, slen, "<JavaScript>", 1, &rval))
    {
        if(!PyErr_Occurred())
        {
            PyErr_SetString(PyExc_RuntimeError, "Failed to execute script.");
        }
        goto error;
    }

    if(PyErr_Occurred()) goto error;

    ret = js2py(self, rval);

    JS_EndRequest(self->cx);
    JS_MaybeGC(self->cx);
    goto success;

error:
    JS_EndRequest(self->cx);
success:

    if(started_counter)
    {
        self->start_time = 0;
    }

    return ret;
}

PyObject*
Context_gc(Context* self, PyObject* args, PyObject* kwargs)
{
    JS_GC(self->cx);
    return (PyObject*) self;
}

PyObject*
Context_max_memory(Context* self, PyObject* args, PyObject* kwargs)
{
    PyObject* ret = NULL;
    long curr_max = -1;
    long new_max = -1;

    if(!PyArg_ParseTuple(args, "|l", &new_max)) goto done;

    curr_max = self->max_heap;
    if(new_max >= 0) self->max_heap = new_max;

    ret = PyLong_FromLong(curr_max);

done:
    return ret;
}

PyObject*
Context_max_time(Context* self, PyObject* args, PyObject* kwargs)
{
    PyObject* ret = NULL;
    int curr_max = -1;
    int new_max = -1;

    if(!PyArg_ParseTuple(args, "|i", &new_max)) goto done;

    curr_max = self->max_time;
    if(new_max > 0) self->max_time = (time_t) new_max;

    ret = PyLong_FromLong((long) curr_max);

done:
    return ret;
}

static PyMemberDef Context_members[] = {
    {NULL}
};

static PyMethodDef Context_methods[] = {
    {
        "add_global",
        (PyCFunction)Context_add_global,
        METH_VARARGS,
        "Install a global object in the JS VM."
    },
    {
        "rem_global",
        (PyCFunction)Context_rem_global,
        METH_VARARGS,
        "Remove a global object in the JS VM."
    },
    {
        "execute",
        (PyCFunction)Context_execute,
        METH_VARARGS,
        "Execute JavaScript source code."
    },
    {
        "gc",
        (PyCFunction)Context_gc,
        METH_VARARGS,
        "Force garbage collection of the JS context."
    },
    {
        "max_memory",
        (PyCFunction)Context_max_memory,
        METH_VARARGS,
        "Get/Set the maximum memory allocation allowed for a context."
    },
    {
        "max_time",
        (PyCFunction)Context_max_time,
        METH_VARARGS,
        "Get/Set the maximum time a context can execute for."
    },
    {NULL}
};

PyTypeObject _ContextType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "spidermonkey.Context",                     /*tp_name*/
    sizeof(Context),                            /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    (destructor)Context_dealloc,                /*tp_dealloc*/
    0,                                          /*tp_print*/
    0,                                          /*tp_getattr*/
    0,                                          /*tp_setattr*/
    0,                                          /*tp_compare*/
    0,                                          /*tp_repr*/
    0,                                          /*tp_as_number*/
    0,                                          /*tp_as_sequence*/
    0,                                          /*tp_as_mapping*/
    0,                                          /*tp_hash*/
    0,                                          /*tp_call*/
    0,                                          /*tp_str*/
    0,                                          /*tp_getattro*/
    0,                                          /*tp_setattro*/
    0,                                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "JavaScript Context",                       /*tp_doc*/
    0,		                                    /*tp_traverse*/
    0,		                                    /*tp_clear*/
    0,		                                    /*tp_richcompare*/
    0,		                                    /*tp_weaklistoffset*/
    0,		                                    /*tp_iter*/
    0,		                                    /*tp_iternext*/
    Context_methods,                            /*tp_methods*/
    Context_members,                            /*tp_members*/
    0,                                          /*tp_getset*/
    0,                                          /*tp_base*/
    0,                                          /*tp_dict*/
    0,                                          /*tp_descr_get*/
    0,                                          /*tp_descr_set*/
    0,                                          /*tp_dictoffset*/
    (initproc)Context_init,                     /*tp_init*/
    0,                                          /*tp_alloc*/
    Context_new,                                /*tp_new*/
};

PyObject*
Context_get_class(Context* cx, const char* key)
{
    return PyDict_GetItemString((PyObject*) cx->classes, key);
}

int
Context_add_class(Context* cx, const char* key, PyObject* val)
{
    return PyDict_SetItemString((PyObject*) cx->classes, key, val);
}

int
Context_has_object(Context* cx, PyObject* val)
{
    return PySet_Contains((PyObject*) cx->objects, val);
}

int
Context_add_object(Context* cx, PyObject* val)
{
    return PySet_Add((PyObject*) cx->objects, val);
}

int
Context_rem_object(Context* cx, PyObject* val)
{
    return PySet_Discard((PyObject*) cx->objects, val);
}
