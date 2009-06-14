/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

PyObject*
Runtime_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    Runtime* self = NULL;
    unsigned int stacksize = 0x2000000; // 32 MiB heap size.

    if(!PyArg_ParseTuple(args, "|I", &stacksize)) goto error;

    self = (Runtime*) type->tp_alloc(type, 0);
    if(self == NULL) goto error;

    self->rt = JS_NewRuntime(stacksize);
    if(self->rt == NULL) goto error;

    goto success;

error:
    Py_XDECREF(self);

success:
    return (PyObject*) self;
}

int
Runtime_init(Runtime* self, PyObject* args, PyObject* kwargs)
{
    return 0;
}

void
Runtime_dealloc(Runtime* self)
{
    if(self->rt != NULL)
    {
        JS_DestroyRuntime(self->rt);
    }
}

PyObject*
Runtime_new_context(Runtime* self, PyObject* args, PyObject* kwargs)
{
    PyObject* cx = NULL;
    PyObject* tpl = NULL;
    PyObject* global = Py_None;
    PyObject* access = Py_None;

    char* keywords[] = {"glbl", "access", NULL};

    if(!PyArg_ParseTupleAndKeywords(
        args, kwargs,
        "|OO",
        keywords,
        &global,
        &access
    )) goto error;

    tpl = Py_BuildValue("OOO", self, global, access);
    if(tpl == NULL) goto error;

    cx = PyObject_CallObject((PyObject*) ContextType, tpl);
    goto success;

error:
    Py_XDECREF(cx);

success:
    Py_XDECREF(tpl);
    return cx;
}

static PyMemberDef Runtime_members[] = {
    {NULL}
};

static PyMethodDef Runtime_methods[] = {
    {
        "new_context",
        (PyCFunction)Runtime_new_context,
        METH_VARARGS | METH_KEYWORDS,
        "Create a new JavaScript Context."
    },
    {NULL}
};

PyTypeObject _RuntimeType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "spidermonkey.Runtime",                     /*tp_name*/
    sizeof(Runtime),                            /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    (destructor)Runtime_dealloc,                /*tp_dealloc*/
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
    "JavaScript Runtime",                       /*tp_doc*/
    0,		                                    /*tp_traverse*/
    0,		                                    /*tp_clear*/
    0,		                                    /*tp_richcompare*/
    0,		                                    /*tp_weaklistoffset*/
    0,		                                    /*tp_iter*/
    0,		                                    /*tp_iternext*/
    Runtime_methods,                            /*tp_methods*/
    Runtime_members,                            /*tp_members*/
    0,                                          /*tp_getset*/
    0,                                          /*tp_base*/
    0,                                          /*tp_dict*/
    0,                                          /*tp_descr_get*/
    0,                                          /*tp_descr_set*/
    0,                                          /*tp_dictoffset*/
    (initproc)Runtime_init,                     /*tp_init*/
    0,                                          /*tp_alloc*/
    Runtime_new,                                /*tp_new*/
};

