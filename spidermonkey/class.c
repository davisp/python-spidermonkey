#include "spidermonkey.h"

JSBool __constructor_cb__(JSContext* cx, JSObject* obj, uintN argc,
                                jsval* argv, jsval* rval);
JSBool __resolve_cb__(JSContext* cx, JSObject* obj, jsval jsv);
JSBool __get_prop_cb__(JSContext* cx, JSObject* obj, jsval jsv, jsval* rval);
JSBool __set_prop_cb__(JSContext* cx, JSObject* obj, jsval jsv, jsval* rval);
void __finalize_cb__(JSContext* cx, JSObject* obj);

PyObject*
Class_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    Class* self;
    PyObject* cx;
    PyObject* pyclass;
    PyTypeObject* pytype;
    JSClass* jsclass = NULL;
    JSObject* proto = NULL;
    PyObject* bind_ctr = Py_True;
    PyObject* is_global = Py_False;
    unsigned int flags = 0;
    
    char* kwnames[] = {"bind_constructor", "is_global", "flags", NULL};
    
    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!|O!O!I", kwnames,
        ContextType, &cx,
        PyClass_Type, &pyclass,
        PyBool_Type, &bind_ctr,
        PyBool_Type, &is_global,
        &flags)
    )
    {
        return NULL;
    }
   
    pytype = pyclass->ob_type;
    
    jsclass = (JSClass*) malloc(sizeof(JSClass));
    if(jsclass == NULL)
    {
        return PyErr_NoMemory();
    }
   
    jsclass->name = (char*) malloc(strlen(pytype->tp_name)*sizeof(char));
    if(jsclass->name != NULL)
    {
        free(jsclass);
        return PyErr_NoMemory();
    }
    
    strcpy((char*) jsclass->name, pytype->tp_name);

    jsclass->flags = flags | JSCLASS_HAS_PRIVATE;
    jsclass->addProperty = JS_PropertyStub;
    jsclass->delProperty = JS_PropertyStub;
    jsclass->getProperty = __get_prop_cb__;
    jsclass->setProperty = __set_prop_cb__;
    jsclass->enumerate = JS_EnumerateStub;
    jsclass->convert = JS_ConvertStub;
    jsclass->finalize = __finalize_cb__;
    jsclass->getObjectOps = NULL;
    jsclass->checkAccess = NULL;
    jsclass->call = NULL;
    jsclass->construct = NULL;
    jsclass->xdrObject = NULL;
    jsclass->hasInstance = NULL;
    jsclass->mark = NULL;
    jsclass->reserveSlots = NULL;

    if(is_global == Py_True)
    {
        jsclass->resolve = __resolve_cb__;
    }
    else
    {
        jsclass->resolve = JS_ResolveStub;
    }
    
    self = (Class*) type->tp_alloc(type, 0);
    if(self == NULL)
    {
        free((char*) jsclass->name);
        free(jsclass);
        return PyErr_NoMemory();
    }
   
    if(bind_ctr == Py_True)
    {
        proto = JS_InitClass(((Context*) cx)->cx, NULL, NULL, jsclass,
                __constructor_cb__, 0, NULL, NULL, NULL, NULL);

        if(proto == NULL)
        {
            free((char*) jsclass->name);
            free(jsclass);
            Py_DECREF(self);
            PyErr_SetString(PyExc_RuntimeError, "Failed to create JS class.");
            return NULL;
        }
    }
    
    Py_INCREF(cx);
    self->cx = cx;
    Py_INCREF(pyclass);
    self->pyclass = pyclass;
    self->jsclass = jsclass;
    self->prototype = proto;

    return (PyObject*) self;
}

int
Class_init(Class* self, PyObject* args, PyObject* kwargs)
{
    return 0;
}

void
Class_dealloc(Class* self)
{
    free((char*) self->jsclass->name);
    free(self->jsclass);
    Py_XDECREF(self->pyclass);
    Py_XDECREF(self->cx);
}

static PyMemberDef Class_members[] = {
    {NULL}
};

static PyMethodDef Class_methods[] = {
    {NULL}
};

PyTypeObject _ClassType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "spidermonkey.Class",                       /*tp_name*/
    sizeof(Class),                              /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    (destructor)Class_dealloc,                  /*tp_dealloc*/
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
    "JavaScript Class Adapter",                 /*tp_doc*/
    0,		                                    /*tp_traverse*/
    0,		                                    /*tp_clear*/
    0,		                                    /*tp_richcompare*/
    0,		                                    /*tp_weaklistoffset*/
    0,		                                    /*tp_iter*/
    0,		                                    /*tp_iternext*/
    Class_methods,                              /*tp_methods*/
    Class_members,                              /*tp_members*/
    0,                                          /*tp_getset*/
    0,                                          /*tp_base*/
    0,                                          /*tp_dict*/
    0,                                          /*tp_descr_get*/
    0,                                          /*tp_descr_set*/
    0,                                          /*tp_dictoffset*/
    (initproc)Class_init,                       /*tp_init*/
    0,                                          /*tp_alloc*/
    Class_new,                                  /*tp_new*/
};

/*
    JavaScript callbacks
*/

JSBool
__constructor_cb__(JSContext* cx, JSObject* obj,
                            uintN argc, jsval* argv, jsval* rval)
{
    return JS_TRUE;
}

JSBool
__resolve_cb__(JSContext* cx, JSObject* obj, jsval jsv)
{
    return JS_TRUE;
}

JSBool
__get_prop_cb__(JSContext* cx, JSObject* obj, jsval jsv, jsval* rval)
{
    return JS_TRUE;
}

JSBool
__set_prop_cb__(JSContext* cx, JSObject* obj, jsval jsv, jsval* rval)
{
    return JS_TRUE;
}

void
__finalize_cb__(JSContext* cx, JSObject* obj)
{
    return;
}

