#include "spidermonkey.h"
#include "libjs/jsobj.h"

static JSClass
js_global_class = {
    "JSGlobalObjectClass",
    JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

PyObject*
Context_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    Context* self;
    Runtime* runtime = NULL;

    if(!PyArg_ParseTuple(args, "O!", RuntimeType, &runtime))
    {
        return NULL;
    }

    self = (Context*) type->tp_alloc(type, 0);
    if(self != NULL)
    {
        // Tracking what classes we've installed in
        // the context.
        self->classes = (PyDictObject*) PyDict_New();
        if(self->classes == NULL)
        {
            Py_DECREF(self);
            return NULL;
        }

        self->objects = (PySetObject*) PySet_New(NULL);
        if(self->objects == NULL)
        {
            Py_DECREF(self->classes);
            Py_DECREF(self);
            return NULL;
        }

        self->cx = JS_NewContext(runtime->rt, 8192);
        if(self->cx == NULL)
        {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create JSContext.");
            Py_DECREF(self);
            return NULL;
        }

        self->root = JS_NewObject(self->cx, &js_global_class, NULL, NULL);
        if(self->root == NULL)
        {
            Py_DECREF(self);
            PyErr_SetString(PyExc_RuntimeError, "Error creating root object.");
            return NULL;
        }

        if(!JS_InitStandardClasses(self->cx, self->root))
        {
            Py_DECREF(self);
            PyErr_SetString(PyExc_RuntimeError, "Error initializing JS VM.");
            return NULL;
        }
      
        /*
            Notice that we aren't ref'ing the python context. If we
            did that'd lead to a lovely cyclic dependancy between
            the JSContext and the PyContext.

            To fix this, any Python object that may be accessed from
            JS code will add a ref to the Python Context to make sure
            it stays alive properly. Hopefully this works.

            Also, if we were doing stuff when the context was destroyed
            we'd have to keep this in mind.
        */
        JS_SetContextPrivate(self->cx, self);
        
        JS_SetErrorReporter(self->cx, report_error_cb);
        
        self->rt = runtime;
        Py_INCREF(self->rt);
    }

    return (PyObject*) self;
}

int
Context_init(Context* self, PyObject* args, PyObject* kwargs)
{
    return 0;
}

void
Context_dealloc(Context* self)
{
    if(self->cx != NULL)
    {
        JS_DestroyContext(self->cx);
    }

    Py_XDECREF(self->objects);
    Py_XDECREF(self->classes);
    Py_DECREF(self->rt);
}

PyObject*
Context_add_global(Context* self, PyObject* args, PyObject* kwargs)
{
    PyObject* pykey = NULL;
    PyObject* pyval = NULL;
    jsval jsk;
    jsid kid;
    jsval jsv;

    if(!PyArg_ParseTuple(args, "OO", &pykey, &pyval))
    {
        return NULL;
    }

    jsk = py2js(self, pykey);
    if(jsk == JSVAL_VOID) return NULL;

    if(!JS_ValueToId(self->cx, jsk, &kid))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to create value id.");
        return NULL;
    }

    jsv = py2js(self, pyval);
    if(jsv == JSVAL_VOID) return NULL;

    if(!js_SetProperty(self->cx, self->root, kid, &jsv))
    {
        PyErr_SetString(PyExc_AttributeError, "Failed to set global property.");
        return NULL;
    }

    Py_RETURN_NONE;
}

PyObject*
Context_execute(Context* self, PyObject* args, PyObject* kwargs)
{
    PyObject* obj = NULL;
    JSString* script = NULL;
    jschar* schars = NULL;
    size_t slen;
    jsval rval;

    if(!PyArg_ParseTuple(args, "O", &obj))
    {
        return NULL;
    }
    
    script = py2js_string_obj(self, obj);
    if(script == NULL) return NULL;
    schars = JS_GetStringChars(script);
    slen = JS_GetStringLength(script);
    
    if(!JS_EvaluateUCScript(self->cx, self->root,
                                schars, slen, "Python", 0, &rval)
    )
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to execute script.");
        return NULL;
    }

    obj = js2py(self, rval);
    JS_MaybeGC(self->cx);
    return obj;
}

static PyMemberDef Context_members[] = {
    {NULL}
};

static PyMethodDef Context_methods[] = {
    {
        "add_global",
        (PyCFunction)Context_add_global,
        METH_VARARGS,
        "Install a global object in the JS VM."
    },
    {
        "execute",
        (PyCFunction)Context_execute,
        METH_VARARGS,
        "Execute JavaScript source code."
    },
    {NULL}
};

PyTypeObject _ContextType = {
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
    "JavaScript Context",                       /*tp_doc*/
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

PyObject*
Context_get_class(Context* cx, const char* key)
{
    return PyDict_GetItemString((PyObject*) cx->classes, key);
}

int
Context_add_class(Context* cx, const char* key, PyObject* val)
{
    return PyDict_SetItemString((PyObject*) cx->classes, key, val);
}

int
Context_has_object(Context* cx, PyObject* val)
{
    return PySet_Contains((PyObject*) cx->objects, val);
}

int
Context_add_object(Context* cx, PyObject* val)
{
    return PySet_Add((PyObject*) cx->objects, val);
}

int
Context_rem_object(Context* cx, PyObject* val)
{
    return PySet_Discard((PyObject*) cx->objects, val);
}
