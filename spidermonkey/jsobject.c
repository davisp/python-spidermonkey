#include "spidermonkey.h"
#include "libjs/jsobj.h"

PyObject*
make_object(PyTypeObject* type, Context* cx, jsval val)
{
    uint32 flags = JSCLASS_HAS_RESERVED_SLOTS(2);
    JSClass* klass = NULL;
    JSObject* obj = NULL;
    jsval priv;
    PyObject* tpl = NULL;
    PyObject* hashable = NULL;
    void* raw = NULL;
    Object* ret = NULL;
    int found;

    // Unwrapping if its wrapped.
    obj = JSVAL_TO_OBJECT(val);
    klass = JS_GetClass(cx->cx, obj);
    if(klass != NULL && (klass->flags & flags) == flags)
    {
        fprintf(stderr, "Checking for is_pyobj\n");
        if(JS_GetReservedSlot(cx->cx, obj, 0, &priv))
        {
            fprintf(stderr, "Got slot\n");
            raw = (PyObject*) JSVAL_TO_PRIVATE(priv);
            hashable = PyCObject_FromVoidPtr(raw, NULL);
            if(hashable == NULL) return NULL;
            fprintf(stderr, "Got CObj\n");

            found = Context_has_object(cx, hashable);
            if(found < 0) return NULL;
            fprintf(stderr, "Found or not\n");
            if(found > 0) return ((PyObject*) raw);
            fprintf(stderr, "NOT FOUND\n");
        }
    }

    // Wrap JS value

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

Py_ssize_t
Object_length(Object* self)
{
    JSContext* cx;
    JSObject* iter;
    jsval pval;
    JSBool status = JS_FALSE;
    Py_ssize_t ret = 0;

    cx = self->cx->cx;
    iter = JS_NewPropertyIterator(cx, self->obj);

    status = JS_NextProperty(cx, self->obj, &pval);
    while(status == JS_TRUE && pval != JSVAL_VOID)
    {
        ret += 1;
        status = JS_NextProperty(cx, self->obj, &pval);
    }

    return ret;
}

PyObject*
Object_getitem(Object* self, PyObject* key)
{
    jsval pval;
    jsid pid;

    pval = py2js(self->cx, key);
    if(pval == JSVAL_VOID) return NULL;
   
    if(!JS_ValueToId(self->cx->cx, pval, &pid))
    {
        PyErr_SetString(PyExc_KeyError, "Failed to get property id.");
        return NULL;
    }
    
    if(!js_GetProperty(self->cx->cx, self->obj, pid, &pval))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to get property.");
        return NULL;
    }

    return js2py_with_parent(self->cx, pval, self->val);
}

int
Object_setitem(Object* self, PyObject* key, PyObject* val)
{
    jsval pval;
    jsval vval;
    jsid pid;

    pval = py2js(self->cx, key);
    if(pval == JSVAL_VOID) return -1;
   
    if(!JS_ValueToId(self->cx->cx, pval, &pid))
    {
        PyErr_SetString(PyExc_KeyError, "Failed to get property id.");
        return -1;
    }
   
    if(val != NULL)
    {
        vval = py2js(self->cx, val);
        if(vval == JSVAL_VOID) return -1;

        if(!js_SetProperty(self->cx->cx, self->obj, pid, &vval))
        {
            PyErr_SetString(PyExc_AttributeError, "Failed to set property.");
            return -1;
        }
    }
    else
    {
        if(!js_DeleteProperty(self->cx->cx, self->obj, pid, &vval))
        {
            PyErr_SetString(PyExc_AttributeError, "Failed to delete property.");
            return -1;
        }

        if(vval == JSVAL_VOID)
        {
            PyErr_SetString(PyExc_AttributeError, "Unable to delete property.");
            return -1;
        }
    }

    return 0;
}

static PyMemberDef Object_members[] = {
    {NULL}
};

static PyMethodDef Object_methods[] = {
    {NULL}
};

PyMappingMethods Object_mapping = {
    (lenfunc)Object_length,                     /*mp_length*/
    (binaryfunc)Object_getitem,                 /*mp_subscript*/
    (objobjargproc)Object_setitem               /*mp_ass_subscript*/
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
    &Object_mapping,                            /*tp_as_mapping*/
    0,                                          /*tp_hash*/
    0,                                          /*tp_call*/
    0,                                          /*tp_str*/
    (getattrofunc)Object_getitem,               /*tp_getattro*/
    (setattrofunc)Object_setitem,               /*tp_setattro*/
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
