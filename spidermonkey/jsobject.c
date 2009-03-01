#include "spidermonkey.h"

PyObject*
make_object(PyTypeObject* type, Context* cx, jsval val)
{
    JSObject* obj = NULL;
    PyObject* tpl = NULL;
    Object* ret = NULL;

    obj = JSVAL_TO_OBJECT(val);

    tpl = Py_BuildValue("(O)", cx);
    if(tpl == NULL)
    {
        return NULL;
    }
    
    ret = (Object*) PyObject_CallObject((PyObject*) type, tpl);
    if(ret != NULL)
    {
        ret->val = val;
        ret->obj = obj;
        if(!JS_AddRoot(cx->cx, &(ret->val)))
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
js2py_object(Context* cx, jsval val)
{
    return make_object(ObjectType, cx, val);
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
        self->val = JSVAL_VOID;
        self->obj = NULL;
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
    if(self->val != JSVAL_VOID)
    {
        JS_RemoveRoot(self->cx->cx, &(self->val));
    }
   
    Py_XDECREF(self->cx);
}

PyObject*
Object_repr(Object* self)
{
    jsval val;
    JSString* repr = NULL;
    jschar* rchars = NULL;
    size_t rlen;
    
    val = OBJECT_TO_JSVAL(self->obj);
    repr = JS_ValueToString(self->cx->cx, val);
    if(repr == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to convert array.");
        return NULL;
    }

    rchars = JS_GetStringChars(repr);
    rlen = JS_GetStringLength(repr);

    return PyUnicode_Decode((const char*) rchars, rlen*2, "utf-16", "strict");
}

PyObject*
Object_getattro(Object* self, PyObject* key)
{
    jsval pval;
    JSType ptype;
    JSString* pobj;
    jschar* pchars;
    size_t plen;
    jsval rval;
    const char* typename;

    pval = py2js(self->cx, key);
    if(pval == JSVAL_VOID) return NULL;
    ptype = JS_TypeOfValue(self->cx->cx, pval);
    
    if(ptype != JSTYPE_STRING)
    {
        typename = JS_GetTypeName(self->cx->cx, ptype);
        return PyErr_Format(PyExc_TypeError, "Invalid type: %s", typename);
    }
    
    pobj = JSVAL_TO_STRING(pval);
    pchars = JS_GetStringChars(pobj);
    plen = JS_GetStringLength(pobj);
    if(!JS_GetUCProperty(self->cx->cx, self->obj, pchars, plen, &rval))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to get property.");
        return NULL;
    }

    return js2py_with_parent(self->cx, rval, self->val);
}

int
Object_setattro(Object* self, PyObject* key, PyObject* val)
{
    jsval pval;
    jsval vval;
    JSType ptype;
    JSString* pobj;
    jschar* pchars;
    size_t plen;
    const char* typename;
    
    pval = py2js(self->cx, key);
    if(pval == JSVAL_VOID) return -1;
    ptype = JS_TypeOfValue(self->cx->cx, pval);

    if(ptype != JSTYPE_STRING)
    {
        typename = JS_GetTypeName(self->cx->cx, ptype);
        PyErr_Format(PyExc_TypeError, "Invalid type: %s", typename);
        return -1;
    }

    vval = py2js(self->cx, val);
    
    pobj = JSVAL_TO_STRING(pval);
    pchars = JS_GetStringChars(pobj);
    plen = JS_GetStringLength(pobj);
    if(!JS_SetUCProperty(self->cx->cx, self->obj, pchars, plen, &vval))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to set property.");
        return -1;
    }

    return 0;
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
    (reprfunc)Object_repr,                      /*tp_repr*/
    0,                                          /*tp_as_number*/
    0,                                          /*tp_as_sequence*/
    0,                                          /*tp_as_mapping*/
    0,                                          /*tp_hash*/
    0,                                          /*tp_call*/
    0,                                          /*tp_str*/
    (getattrofunc)Object_getattro,              /*tp_getattro*/
    (setattrofunc)Object_setattro,              /*tp_setattro*/
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
