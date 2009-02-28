#include "spidermonkey.h"

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

    Py_DECREF(self->rt);
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
    
    script = py2js_string(self->cx, obj);
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
    
    obj = js2py_string(JS_ValueToString(self->cx, rval));
    if(obj == NULL) return NULL;
    
    PyObject_Print(obj, stderr, 0);
    fprintf(stderr, "\n");
  
    Py_RETURN_NONE;
}

static PyMemberDef Context_members[] = {
    {NULL}
};

static PyMethodDef Context_methods[] = {
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
