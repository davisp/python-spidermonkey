/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"
#include "libjs/jsobj.h"

static JSClass
js_global_class = {
    "JSGlobalObjectClass",
    JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

PyObject*
Context_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    Context* self = NULL;
    Runtime* runtime = NULL;

    if(!PyArg_ParseTuple(args, "O!", RuntimeType, &runtime)) goto error;

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
    JS_SetErrorReporter(self->cx, report_error_cb);
    
    Py_INCREF(runtime);
    self->rt = runtime;

    goto success;

error:
    Py_XDECREF(self);

success:
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

    Py_XDECREF(self->objects);
    Py_XDECREF(self->classes);
    Py_DECREF(self->rt);
}

PyObject*
Context_add_global(Context* self, PyObject* args, PyObject* kwargs)
{
    PyObject* pykey = NULL;
    PyObject* pyval = NULL;
    jsval jsk;
    jsid kid;
    jsval jsv;

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
    size_t slen;
    jsval rval;

    if(!PyArg_ParseTuple(args, "O", &obj)) goto error;
    
    script = py2js_string_obj(self, obj);
    if(script == NULL) goto error;

    schars = JS_GetStringChars(script);
    slen = JS_GetStringLength(script);
    
    cx = self->cx;
    root = self->root;
    
    if(!JS_EvaluateUCScript(cx, root, schars, slen, "<JavaScript>", 1, &rval))
    {
        if(!PyErr_Occurred())
        {
            PyErr_SetString(PyExc_RuntimeError, "Failed to execute script.");
        }
        goto error;
    }

    if(PyErr_Occurred())
    {
        PyErr_PrintEx(0);
        exit(-1);
    }

    ret = js2py(self, rval);
    JS_MaybeGC(self->cx);
    goto success;

error:
success:
    return ret;
}

PyObject*
Context_gc(Context* self, PyObject* args, PyObject* kwargs)
{
    JS_GC(self->cx);
    return (PyObject*) self;
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
