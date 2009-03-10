/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

PyObject*
js2py_array(Context* cx, jsval val)
{
    return make_object(ArrayType, cx, val);
}

Py_ssize_t
Array_length(Object* self)
{
    jsuint length;
    if(!JS_GetArrayLength(self->cx->cx, self->obj, &length))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to get array length.");
        return -1;
    }

    return (Py_ssize_t) length;
}

PyObject*
Array_get_item(Object* self, Py_ssize_t idx)
{
    jsval rval;
    jsint pos = (jsint) idx;

    if(!JS_GetElement(self->cx->cx, self->obj, pos, &rval))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to get array item.");
        return NULL;
    }

    return js2py(self->cx, rval);
}

int
Array_set_item(Object* self, Py_ssize_t idx, PyObject* val)
{
    jsval pval;
    jsint pos = (jsint) idx;

    pval = py2js(self->cx, val);
    if(pval == JSVAL_VOID) return -1;

    if(!JS_SetElement(self->cx->cx, self->obj, pos, &pval))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to set array item.");
        return -1;
    }

    return 0;
}

static PyMemberDef Array_members[] = {
    {0, 0, 0, 0}
};

static PyMethodDef Array_methods[] = {
    {0, 0, 0, 0}
};

static PySequenceMethods Array_seq_methods = {
    (lenfunc)Array_length,                      /*sq_length*/
    0,                                          /*sq_concat*/
    0,                                          /*sq_repeat*/
    (ssizeargfunc)Array_get_item,               /*sq_item*/
    0,                                          /*sq_slice*/
    (ssizeobjargproc)Array_set_item,            /*sq_ass_item*/
    0,                                          /*sq_ass_slice*/
    0,                                          /*sq_contains*/
    0,                                          /*sq_inplace_concat*/
    0,                                          /*sq_inplace_repeat*/
};

PyTypeObject _ArrayType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "spidermonkey.Array",                       /*tp_name*/
    sizeof(Object),                             /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    0,                                          /*tp_dealloc*/
    0,                                          /*tp_print*/
    0,                                          /*tp_getattr*/
    0,                                          /*tp_setattr*/
    0,                                          /*tp_compare*/
    0,                                          /*tp_repr*/
    0,                                          /*tp_as_number*/
    &Array_seq_methods,                         /*tp_as_sequence*/
    0,                                          /*tp_as_mapping*/
    0,                                          /*tp_hash*/
    0,                                          /*tp_call*/
    0,                                          /*tp_str*/
    0,                                          /*tp_getattro*/
    0,                                          /*tp_setattro*/
    0,                                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "JavaScript Array",                         /*tp_doc*/
    0,		                                    /*tp_traverse*/
    0,		                                    /*tp_clear*/
    0,		                                    /*tp_richcompare*/
    0,		                                    /*tp_weaklistoffset*/
    0,		                                    /*tp_iter*/
    0,		                                    /*tp_iternext*/
    Array_methods,                              /*tp_methods*/
    Array_members,                              /*tp_members*/
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
