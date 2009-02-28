#ifndef PYSM_CONTEXT
#define PYSM_CONTEXT

#include <Python.h>
#include "structmember.h"

#include "libjs/jsapi.h"

typedef struct {
    PyObject_HEAD
    PyObject* root;
    JSContext* cx;
} Context;

PyObject*
    Context_new(PyTypeObject* type, PyObject* args, PyObject* kwargs);
int Context_init(Context* self, PyObject* args, PyObject* kwargs);
void Context_dealloc(Context* self);

static PyMemberDef Context_members[] = {
    {NULL}
};

static PyMethodDef Context_methods[] = {
    {NULL}
};

static PyTypeObject ContextType = {
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
    "JavaScript Context.\n\nspidermonkey.Context(runtime_inst, root=None)",
                                                /*tp_doc*/
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

#endif
