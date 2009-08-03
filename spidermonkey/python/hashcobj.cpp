/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include <spidermonkey.h>

PyObject*
HashCObj_FromVoidPtr(void *cobj)
{
    HashCObj* self = PyObject_NEW(HashCObj, HashCObjType);
    if(self == NULL) return NULL;
    self->cobj = cobj;
    return (PyObject*) self;
}

void*
HashCObj_AsVoidPtr(PyObject* self)
{
    return ((HashCObj*)self)->cobj;
}

int
HashCObj_cmp(PyObject* self, PyObject* other)
{
    int ret = -1;

    if(!PyObject_TypeCheck(self, HashCObjType))
    {
        PyErr_SetString(PyExc_ValueError, "Invalid comparison object.");
        return -1;
    }

    if(!PyObject_TypeCheck(other, HashCObjType))
    {
        PyErr_SetString(PyExc_ValueError, "Invalid comparison object 2.");
        return -1;
    }
    
    if(((HashCObj*)self)->cobj == ((HashCObj*)other)->cobj)
        return 0;
    else
        return 1;
}

PyObject*
HashCObj_repr(PyObject* self)
{
    return PyString_FromFormat(
        "<%s Ptr: %p>",
        self->ob_type->tp_name,
        ((HashCObj*)self)->cobj
    );
}

long
HashCObj_hash(HashCObj* self)
{
    return _Py_HashPointer(self->cobj);
}

PyTypeObject _HashCObjType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "spidermonkey._HashCObj",                   /*tp_name*/
    sizeof(HashCObj),                           /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    0,                                          /*tp_dealloc*/
    0,                                          /*tp_print*/
    0,                                          /*tp_getattr*/
    0,                                          /*tp_setattr*/
    (cmpfunc)HashCObj_cmp,                      /*tp_compare*/
    (reprfunc)HashCObj_repr,                    /*tp_repr*/
    0,                                          /*tp_as_number*/
    0,                                          /*tp_as_sequence*/
    0,                                          /*tp_as_mapping*/
    (hashfunc)HashCObj_hash,                    /*tp_hash*/
    0,                                          /*tp_call*/
    0,                                          /*tp_str*/
    0,                                          /*tp_getattro*/
    0,                                          /*tp_setattro*/
    0,                                          /*tp_as_buffer*/
    0,                                          /*tp_flags*/
    "Internal hashing object.",                 /*tp_doc*/
};
