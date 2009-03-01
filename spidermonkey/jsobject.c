#include "spidermonkey.h"

PyObject*
js2py_object(Context* cx, jsval val)
{
    JSObject* obj = NULL;
    PyObject* tpl = NULL;
    Object* ret = NULL;

    obj = JSVAL_TO_OBJECT(val);

    tpl = Py_BuildValue("(O)", cx);
    if(tpl == NULL)
    {
        JS_RemoveRoot(cx->cx, &obj);
        return NULL;
    }
    
    ret = (Object*) PyObject_CallObject((PyObject*) ObjectType, tpl);
    if(ret != NULL)
    {
        ret->jsobj = obj;
        if(!JS_AddRoot(cx->cx, &(ret->jsobj)))
        {
            Py_DECREF(ret);
            PyErr_SetString(PyExc_RuntimeError, "Failed to set GC root.");
            ret = NULL;
        }
    }

    Py_DECREF(tpl);
    
    return (PyObject*) ret;
}

PyObject*
Object_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    Object* self;
    Context* cx;
    
    if(!PyArg_ParseTuple(args, "O!", ContextType, &cx))
    {
        return NULL;
    }

    self = (Object*) type->tp_alloc(type, 0);
    if(self != NULL)
    {
        Py_INCREF(cx);
        self->cx = cx;
        self->jsobj = NULL;
    }

    return (PyObject*) self;
}

int
Object_init(Object* self, PyObject* args, PyObject* kwargs)
{
    return 0;
}

void
Object_dealloc(Object* self)
{
    if(self->jsobj != NULL)
    {
        JS_RemoveRoot(self->cx->cx, &(self->jsobj));
    }
   
    Py_XDECREF(self->cx);
}

static PyMemberDef Object_members[] = {
    {NULL}
};

static PyMethodDef Object_methods[] = {
    {NULL}
};

PyTypeObject _ObjectType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "spidermonkey.Object",                      /*tp_name*/
    sizeof(Object),                             /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    (destructor)Object_dealloc,                 /*tp_dealloc*/
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
    "JavaScript Object",                        /*tp_doc*/
    0,		                                    /*tp_traverse*/
    0,		                                    /*tp_clear*/
    0,		                                    /*tp_richcompare*/
    0,		                                    /*tp_weaklistoffset*/
    0,		                                    /*tp_iter*/
    0,		                                    /*tp_iternext*/
    Object_methods,                             /*tp_methods*/
    Object_members,                             /*tp_members*/
    0,                                          /*tp_getset*/
    0,                                          /*tp_base*/
    0,                                          /*tp_dict*/
    0,                                          /*tp_descr_get*/
    0,                                          /*tp_descr_set*/
    0,                                          /*tp_dictoffset*/
    (initproc)Object_init,                      /*tp_init*/
    0,                                          /*tp_alloc*/
    Object_new,                                 /*tp_new*/
};
