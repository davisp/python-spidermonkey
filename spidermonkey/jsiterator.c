/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

PyObject*
Iterator_Wrap(Context* cx, JSObject* obj)
{
    Iterator* self = NULL;
    PyObject* tpl = NULL;
    PyObject* ret = NULL;

    JS_BeginRequest(cx->cx);

    // Build our new python object.
    tpl = Py_BuildValue("(O)", cx);
    if(tpl == NULL) goto error;
    
    self = (Iterator*) PyObject_CallObject((PyObject*) IteratorType, tpl);
    if(self == NULL) goto error;
    
    // Attach a JS property iterator.
    self->iter = JS_NewPropertyIterator(cx->cx, obj);
    if(self->iter == NULL) goto error;

    self->root = OBJECT_TO_JSVAL(self->iter);
    if(!JS_AddRoot(cx->cx, &(self->root)))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to set GC root.");
        goto error;
    }

    ret = (PyObject*) self;
    goto success;

error:
    Py_XDECREF(self);
    ret = NULL; // In case it was AddRoot
success:
    Py_XDECREF(tpl);
    JS_EndRequest(cx->cx);
    return (PyObject*) ret;
}

PyObject*
Iterator_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    Context* cx = NULL;
    Iterator* self = NULL;

    if(!PyArg_ParseTuple(args, "O!", ContextType, &cx)) goto error;

    self = (Iterator*) type->tp_alloc(type, 0);
    if(self == NULL) goto error;
    
    Py_INCREF(cx);
    self->cx = cx;
    self->iter = NULL;
    goto success;

error:
    ERROR("spidermonkey.Iterator.new");
success:
    return (PyObject*) self;
}

int
Iterator_init(Iterator* self, PyObject* args, PyObject* kwargs)
{
    return 0;
}

void
Iterator_dealloc(Iterator* self)
{
    if(self->iter != NULL)
    {
        JS_BeginRequest(self->cx->cx);
        JS_RemoveRoot(self->cx->cx, &(self->root));
        JS_EndRequest(self->cx->cx);
    }
   
    Py_XDECREF(self->cx);
}

PyObject*
Iterator_next(Iterator* self)
{
    PyObject* ret = NULL;
    jsid propid;
    jsval propname;

    JS_BeginRequest(self->cx->cx);

    if(!JS_NextProperty(self->cx->cx, self->iter, &propid))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to iterate next JS prop.");
        goto done;
    }

    if(!JS_IdToValue(self->cx->cx, propid, &propname))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to convert property id.");
        goto done;
    }

    if(propname != JSVAL_VOID)
    {
        ret = js2py(self->cx, propname);
    }

    // We return NULL with no error to signal completion.

done:
    JS_EndRequest(self->cx->cx);
    return ret;
}

static PyMemberDef Iterator_members[] = {
    {NULL}
};

static PyMethodDef Iterator_methods[] = {
    {NULL}
};

PyTypeObject _IteratorType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "spidermonkey.Iterator",                    /*tp_name*/
    sizeof(Iterator),                           /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    (destructor)Iterator_dealloc,               /*tp_dealloc*/
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
    "JavaScript Iterator",                      /*tp_doc*/
    0,		                                    /*tp_traverse*/
    0,		                                    /*tp_clear*/
    0,                                          /*tp_richcompare*/
    0,		                                    /*tp_weaklistoffset*/
    0,		                                    /*tp_iter*/
    (iternextfunc)Iterator_next,		        /*tp_iternext*/
    Iterator_methods,                           /*tp_methods*/
    Iterator_members,                           /*tp_members*/
    0,                                          /*tp_getset*/
    0,                                          /*tp_base*/
    0,                                          /*tp_dict*/
    0,                                          /*tp_descr_get*/
    0,                                          /*tp_descr_set*/
    0,                                          /*tp_dictoffset*/
    (initproc)Iterator_init,                    /*tp_init*/
    0,                                          /*tp_alloc*/
    Iterator_new,                               /*tp_new*/
};
