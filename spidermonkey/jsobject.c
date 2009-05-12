/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"
#include <jsobj.h>

PyObject*
make_object(PyTypeObject* type, Context* cx, jsval val)
{
    Object* wrapped = NULL;
    PyObject* tpl = NULL;
    PyObject* hashable = NULL;
    PyObject* ret = NULL;
    void* raw = NULL;
    uint32 flags = JSCLASS_HAS_RESERVED_SLOTS(1);
    JSClass* klass = NULL;
    JSObject* obj = NULL;
    jsval priv;
    int found;

    JS_BeginRequest(cx->cx);

    // Unwrapping if its wrapped.
    obj = JSVAL_TO_OBJECT(val);
    klass = JS_GetClass(cx->cx, obj);
    if(klass != NULL && (klass->flags & flags) == flags)
    {
        if(JS_GetReservedSlot(cx->cx, obj, 0, &priv))
        {
            raw = (PyObject*) JSVAL_TO_PRIVATE(priv);
            hashable = HashCObj_FromVoidPtr(raw);
            if(hashable == NULL) goto error;

            found = Context_has_object(cx, hashable);
            if(found < 0) goto error;
            if(found > 0)
            {
                ret = (PyObject*) raw;
                Py_INCREF(ret);
                goto success;
            }
        }
    }

    // Wrap JS value
    tpl = Py_BuildValue("(O)", cx);
    if(tpl == NULL) goto error;
    
    wrapped = (Object*) PyObject_CallObject((PyObject*) type, tpl);
    if(wrapped == NULL) goto error;
    
    wrapped->val = val;
    wrapped->obj = obj;

    if(!JS_AddRoot(cx->cx, &(wrapped->val)))
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to set GC root.");
        goto error;
    }

    ret = (PyObject*) wrapped;
    goto success;

error:
    Py_XDECREF(wrapped);
    ret = NULL; // In case it was AddRoot

success:
    Py_XDECREF(tpl);
    JS_EndRequest(cx->cx);
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
    Object* self = NULL;
    Context* cx = NULL;

    if(!PyArg_ParseTuple(args, "O!", ContextType, &cx)) goto error;

    self = (Object*) type->tp_alloc(type, 0);
    if(self == NULL) goto error;
    
    Py_INCREF(cx);
    self->cx = cx;
    self->val = JSVAL_VOID;
    self->obj = NULL;
    goto success;

error:
    ERROR("spidermonkey.Object.new");
success:
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
        JS_BeginRequest(self->cx->cx);
        JS_RemoveRoot(self->cx->cx, &(self->val));
        JS_EndRequest(self->cx->cx);
    }
   
    Py_XDECREF(self->cx);
}

PyObject*
Object_repr(Object* self)
{
    //jsval val;
    JSString* repr = NULL;
    jschar* rchars = NULL;
    size_t rlen;
    
    JS_BeginRequest(self->cx->cx);

    repr = JS_ValueToString(self->cx->cx, self->val);
    if(repr == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to convert to a string.");
        JS_EndRequest(self->cx->cx);
        return NULL;
    }

    rchars = JS_GetStringChars(repr);
    rlen = JS_GetStringLength(repr);

    JS_EndRequest(self->cx->cx);
    return PyUnicode_Decode((const char*) rchars, rlen*2, "utf-16", "strict");
}

Py_ssize_t
Object_length(Object* self)
{
    JSContext* cx;
    JSObject* iter;
    jsid pid;
    JSBool status = JS_FALSE;
    Py_ssize_t ret = 0;

    /*
        Using an iterator to make sure we get all
        the properties for this object and its
        prototype as per JS for ... in ... semantics.
    */
    
    JS_BeginRequest(self->cx->cx);
    cx = self->cx->cx;
    iter = JS_NewPropertyIterator(cx, self->obj);
    status = JS_NextProperty(cx, iter, &pid);
    while(status == JS_TRUE && pid != JSVAL_VOID)
    {
        ret += 1;
        status = JS_NextProperty(cx, iter, &pid);
    }

    JS_EndRequest(self->cx->cx);
    return ret;
}

PyObject*
Object_getitem(Object* self, PyObject* key)
{
    PyObject* ret = NULL;
    jsval pval;
    jsid pid;

    JS_BeginRequest(self->cx->cx);

    pval = py2js(self->cx, key);
    if(pval == JSVAL_VOID) return NULL;
   
    if(!JS_ValueToId(self->cx->cx, pval, &pid))
    {
        PyErr_SetString(PyExc_KeyError, "Failed to get property id.");
        goto done;
    }
    
    if(!js_GetProperty(self->cx->cx, self->obj, pid, &pval))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to get property.");
        goto done;
    }

    ret = js2py_with_parent(self->cx, pval, self->val);

done:
    JS_EndRequest(self->cx->cx);
    return ret;
}

int
Object_setitem(Object* self, PyObject* key, PyObject* val)
{
    int ret = -1;
    jsval pval;
    jsval vval;
    jsid pid;

    JS_BeginRequest(self->cx->cx);

    pval = py2js(self->cx, key);
    if(pval == JSVAL_VOID) goto done;
   
    if(!JS_ValueToId(self->cx->cx, pval, &pid))
    {
        PyErr_SetString(PyExc_KeyError, "Failed to get property id.");
        goto done;
    }
   
    if(val != NULL)
    {
        vval = py2js(self->cx, val);
        if(vval == JSVAL_VOID) goto done;

        if(!js_SetProperty(self->cx->cx, self->obj, pid, &vval))
        {
            PyErr_SetString(PyExc_AttributeError, "Failed to set property.");
            goto done;
        }
    }
    else
    {
        if(!js_DeleteProperty(self->cx->cx, self->obj, pid, &vval))
        {
            PyErr_SetString(PyExc_AttributeError, "Failed to delete property.");
            goto done;
        }

        if(vval == JSVAL_VOID)
        {
            PyErr_SetString(PyExc_AttributeError, "Unable to delete property.");
            goto done;
        }
    }

    ret = 0;
done:
    JS_EndRequest(self->cx->cx);
    return ret;
}

PyObject*
Object_rich_cmp(Object* self, PyObject* other, int op)
{
    PyObject* key = NULL;
    PyObject* val = NULL;
    PyObject* otherval = NULL;
    PyObject* ret = NULL;
    JSContext* cx;
    JSObject* iter;
    JSBool status = JS_FALSE;
    jsid pid;
    jsval pkey;
    jsval pval;
    int llen;
    int rlen;
    int cmp;

    JS_BeginRequest(self->cx->cx);

    if(!PyMapping_Check(other) && !PySequence_Check(other))
    {
        PyErr_SetString(PyExc_ValueError, "Invalid rhs operand.");
        goto error;
    }

    if(op != Py_EQ && op != Py_NE) return Py_NotImplemented;

    llen = PyObject_Length((PyObject*)self);
    if(llen < 0) goto error;

    rlen = PyObject_Length(other);
    if(rlen < 0) goto error;

    if(llen != rlen)
    {
        if(op == Py_EQ) ret = Py_False;
        else ret = Py_True;
        goto success;
    }

    cx = self->cx->cx;
    iter = JS_NewPropertyIterator(cx, self->obj);
    status = JS_NextProperty(cx, iter, &pid);
    while(status == JS_TRUE && pid != JSVAL_VOID)
    {
        if(!JS_IdToValue(self->cx->cx, pid, &pkey))
        {
            PyErr_SetString(PyExc_RuntimeError, "Failed to get key.");
            goto error;
        }

        if(!js_GetProperty(self->cx->cx, self->obj, pid, &pval))
        {
            PyErr_SetString(PyExc_RuntimeError, "Failed to get property.");
            goto error;
        }

        key = js2py(self->cx, pkey);
        if(key == NULL) goto error;

        val = js2py(self->cx, pval);
        if(val == NULL) goto error;

        otherval = PyObject_GetItem(other, key);
        if(otherval == NULL)
        {
            PyErr_Clear();
            if(op == Py_EQ) ret = Py_False;
            else ret = Py_True;
            goto success;
        }

        cmp = PyObject_Compare(val, otherval);
        if(PyErr_Occurred()) goto error;

        if(cmp != 0)
        {
            if(op == Py_EQ) ret = Py_False;
            else ret = Py_True;
            goto success;
        }

        Py_DECREF(key); key = NULL;
        Py_DECREF(val); val = NULL;
        Py_DECREF(otherval); otherval = NULL;

        status = JS_NextProperty(cx, iter, &pid);
    }

    if(op == Py_EQ) ret = Py_True;
    else ret = Py_False;

    goto success;

error:
success:
    Py_XDECREF(key);
    Py_XDECREF(val);
    Py_XDECREF(otherval);
    // Inc ref the true or false return
    if(ret == Py_True || ret == Py_False) Py_INCREF(ret);
    JS_EndRequest(self->cx->cx);
    return ret;
}

PyObject*
Object_iterator(Object* self, PyObject* args, PyObject* kwargs)
{
    return Iterator_Wrap(self->cx, self->obj);
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
    (richcmpfunc)Object_rich_cmp,		        /*tp_richcompare*/
    0,		                                    /*tp_weaklistoffset*/
    (getiterfunc)Object_iterator,		        /*tp_iter*/
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
