/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

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

void
Function_dealloc(Function* self)
{
    if(self->parent != JSVAL_VOID)
    {
        JS_BeginRequest(self->obj.cx->cx);
        JS_RemoveRoot(self->obj.cx->cx, &(self->parent));
        JS_EndRequest(self->obj.cx->cx);
    }

    ObjectType->tp_dealloc((PyObject*) self);
}

PyObject*
Function_call(Function* self, PyObject* args, PyObject* kwargs)
{
    PyObject* item = NULL;
    PyObject* ret = NULL;
    Py_ssize_t argc;
    Py_ssize_t idx;
    JSContext* cx;
    JSObject* parent;
    jsval func;
    jsval* argv = NULL;
    jsval rval;
    JSBool started_counter = JS_FALSE;

    JS_BeginRequest(self->obj.cx->cx);

    argc = PySequence_Length(args);
    if(argc < 0) goto error;
    
    argv = malloc(sizeof(jsval) * argc);
    if(argv == NULL)
    {
        PyErr_NoMemory();
        goto error;
    }
    
    for(idx = 0; idx < argc; idx++)
    {
        item = PySequence_GetItem(args, idx);
        if(item == NULL) goto error;
        
        argv[idx] = py2js(self->obj.cx, item);
        if(argv[idx] == JSVAL_VOID) goto error;
        Py_DECREF(item);
        item = NULL; // Prevent double decref.
    }

    func = self->obj.val;
    cx = self->obj.cx->cx;
    parent = JSVAL_TO_OBJECT(self->parent);

    // Mark us for execution time if not already marked
    if(self->obj.cx->start_time == 0)
    {
        started_counter = JS_TRUE;
        self->obj.cx->start_time = time(NULL);
    }

    if(!JS_CallFunctionValue(cx, parent, func, argc, argv, &rval))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to execute JS Function.");
        goto error;
    }

    ret = js2py(self->obj.cx, rval);
    JS_EndRequest(self->obj.cx->cx);
    JS_MaybeGC(cx);
    goto success;

error:
    if(argv != NULL) free(argv);
    JS_EndRequest(self->obj.cx->cx);
success:

    // Reset the time counter if we started it.
    if(started_counter)
    {
        self->obj.cx->start_time = 0;
    }

    Py_XDECREF(item);
    return ret;
}

static PyMemberDef Function_members[] = {
    {NULL}
};

static PyMethodDef Function_methods[] = {
    {NULL}
};

PyTypeObject _FunctionType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "spidermonkey.Function",                    /*tp_name*/
    sizeof(Function),                           /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    (destructor)Function_dealloc,               /*tp_dealloc*/
    0,                                          /*tp_print*/
    0,                                          /*tp_getattr*/
    0,                                          /*tp_setattr*/
    0,                                          /*tp_compare*/
    0,                                          /*tp_repr*/
    0,                                          /*tp_as_number*/
    0,                                          /*tp_as_sequence*/
    0,                                          /*tp_as_mapping*/
    0,                                          /*tp_hash*/
    (ternaryfunc)Function_call,                 /*tp_call*/
    0,                                          /*tp_str*/
    0,                                          /*tp_getattro*/
    0,                                          /*tp_setattro*/
    0,                                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "JavaScript Function",                      /*tp_doc*/
    0,		                                    /*tp_traverse*/
    0,		                                    /*tp_clear*/
    0,		                                    /*tp_richcompare*/
    0,		                                    /*tp_weaklistoffset*/
    0,		                                    /*tp_iter*/
    0,		                                    /*tp_iternext*/
    Function_methods,                           /*tp_methods*/
    Function_members,                           /*tp_members*/
    0,                                          /*tp_getset*/
    0,                                          /*tp_base*/
    0,                                          /*tp_dict*/
    0,                                          /*tp_descr_get*/
    0,                                          /*tp_descr_set*/
    0,                                          /*tp_dictoffset*/
    0,                                          /*tp_init*/
    0,                                          /*tp_alloc*/
    0,                                          /*tp_new*/
};
