/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include <spidermonkey.h>
#include <jsobj.h>

PyObject* get_py_obj(JSContext* cx, JSObject* obj);
PyObject* mk_args_tpl(Context* pycx, JSContext* jscx, uintN argc, jsval* argv);

JSBool
js_add_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* val)
{
    return JS_TRUE;
}

JSBool
js_del_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");
    
    PyObject* pyobj = get_py_obj(jscx, jsobj);
    if(!pyobj) return js_error(jscx, "Failed to get Python object.");
    
    PyObjectXDR pykey = js2py(pycx, key);
    if(!pykey) return js_error(jscx, "Failed to covert object key.");

    if(Context_has_access(pycx, jscx, pyobj, pykey.get()) <= 0)
        return js_error(jscx, "Access denied.");

    if(PyObject_DelItem(pyobj, pykey.get()) < 0)
    {
        PyErr_Clear();
        if(PyObject_DelAttr(pyobj, pykey.get()) < 0)
        {
            PyErr_Clear();
            *rval = JSVAL_FALSE;
        }
    }
   
    return JS_TRUE;
}

JSBool
js_get_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");

    PyObject* pyobj = get_py_obj(jscx, jsobj);
    if(!pyobj) return js_error(jscx, "Failed to get Python object.");
    
    PyObjectXDR pykey = js2py(pycx, key);
    if(!pykey) return js_error(jscx, "Failed to convert key.");
    
    if(Context_has_access(pycx, jscx, pyobj, pykey.get()) <= 0)
        return js_error(jscx, "Access denied.");
    
    // Yeah. It's ugly as sin.
    if(PyString_Check(pykey.get()) || PyUnicode_Check(pykey.get()))
    {
        PyObjectXDR utf8 = PyUnicode_AsUTF8String(pykey.get());
        if(!utf8) return js_error(jscx, "Failed to convert string to UTF-8");

        const char* data = PyString_AsString(utf8.get());
        if(data == NULL)
            return js_error(jscx, "Failed to convert string to buffer.");

        if(strcmp("__iterator__", data) == 0)
        {
            if(!new_py_iter(pycx, pyobj, rval))
                return JS_FALSE;
            
            if(*rval != JSVAL_VOID)
                return JS_TRUE;

            return JS_FALSE;
        }
    }

    PyObjectXDR pyval = PyObject_GetItem(pyobj, pykey.get());
    if(!pyval)
    {        
        PyErr_Clear();
        pyval = PyObject_GetAttr(pyobj, pykey.get());
        if(!pyval)
        {
            PyErr_Clear();
            *rval = JSVAL_VOID;
            return JS_TRUE;
        }
    }

    *rval = py2js(pycx, pyval.get());
    if(*rval == JSVAL_VOID)
        return JS_FALSE;
    
    return JS_TRUE;
}

JSBool
js_set_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* val)
{
    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");
    
    PyObject* pyobj = get_py_obj(jscx, jsobj);
    if(!pyobj) return js_error(jscx, "Failed to find a Python object.");
    
    PyObjectXDR pykey = js2py(pycx, key);
    if(!pykey) return js_error(jscx, "Failed to convert key to Python.");
    
    if(Context_has_access(pycx, jscx, pyobj, pykey.get()) <= 0)
        return js_error(jscx, "Access denied.");
    
    PyObjectXDR pyval = js2py(pycx, *val);
    if(!pyval) return js_error(jscx, "Failed to convert value to Python.");

    if(PyObject_SetItem(pyobj, pykey.get(), pyval.get()) < 0)
    {
        PyErr_Clear();
        if(PyObject_SetAttr(pyobj, pykey.get(), pyval.get()) < 0)
            return js_error(jscx, "Failed to set Python property.");
    }

    return JS_TRUE;
}

void
js_finalize(JSContext* jscx, JSObject* jsobj)
{
    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx)
    {
        // Not much else we can do but yell.
        fprintf(stderr, "*** NO PYTHON CONTEXT ***\n");
        return;
    }

    JSRequest req(jscx);
    PyObject* pyobj = get_py_obj(jscx, jsobj);
    Py_DECREF(pyobj);
}

JSBool
js_call(JSContext* jscx, JSObject* jsobj, uintN argc, jsval* argv, jsval* rval)
{
    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");

    PyObject* pyobj = get_py_obj(jscx, JSVAL_TO_OBJECT(argv[-2]));    
    if(!PyCallable_Check(pyobj))
        return js_error(jscx, "Object is not callable.");

    // Use '__call__' as a notice that we want to execute a function.
    PyObjectXDR attrcheck = PyString_FromString("__call__");
    if(!attrcheck) return js_error(jscx, "Failed to create attribute check.");

    if(Context_has_access(pycx, jscx, pyobj, attrcheck.get()) <= 0)
        return js_error(jscx, "Access denied.");

    PyObjectXDR tpl = mk_args_tpl(pycx, jscx, argc, argv);
    if(!tpl) return js_error(jscx, "Failed to create function arguments.");
    
    PyObjectXDR ret = PyObject_Call(pyobj, tpl.get(), NULL);
    if(!ret)
    {
        if(!PyErr_Occurred())
        {
            PyErr_SetString(PyExc_RuntimeError, "Failed to call object.");
        }
        
        return js_error(jscx, "Failed to call object.");
    }
    
    *rval = py2js(pycx, ret.get());
    if(*rval == JSVAL_VOID)
        return js_error(jscx, "Failed to convert Python return value.");

    return JS_TRUE;
}

JSBool
js_ctor(JSContext* jscx, JSObject* jsobj, uintN argc, jsval* argv, jsval* rval)
{
    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");
    
    PyObject* pyobj = get_py_obj(jscx, JSVAL_TO_OBJECT(argv[-2]));
    if(!PyCallable_Check(pyobj))
        return js_error(jscx, "Object is not callable.");

    if(!PyType_Check(pyobj))
        return js_error(jscx, "Object is not a Python type.");

    // Use '__init__' to signal use as a constructor.
    PyObjectXDR attrcheck = PyString_FromString("__init__");
    if(!attrcheck) return js_error(jscx, "Failed to create attribute check.");

    if(Context_has_access(pycx, jscx, pyobj, attrcheck.get()) <= 0)
        return js_error(jscx, "Access denied.");

    PyObjectXDR tpl = mk_args_tpl(pycx, jscx, argc, argv);
    if(!tpl) return js_error(jscx, "Failed to create function arguments.");
    
    PyObjectXDR ret = PyObject_CallObject(pyobj, tpl.get());
    if(!ret) return js_error(jscx, "Failed to construct new instance.");
    
    *rval = py2js(pycx, ret.get());
    if(*rval == JSVAL_VOID)
        return js_error(jscx, "Failed to convert new Python instance.");

    return JS_TRUE;
}

JSClass*
create_class(Context* cx, PyObject* pyobj)
{
    PyObject* curr = NULL;
    JSClass* jsclass = NULL;
    JSClass* ret = NULL;
    char* classname = NULL;
    int flags = JSCLASS_HAS_RESERVED_SLOTS(1);

    curr = Context_get_class(cx, pyobj->ob_type->tp_name);
    if(curr != NULL) return (JSClass*) HashCObj_AsVoidPtr(curr);

    jsclass = (JSClass*) malloc(sizeof(JSClass));
    if(jsclass == NULL)
    {
        PyErr_NoMemory();
        goto error;
    }
   
    classname = (char*) malloc(strlen(pyobj->ob_type->tp_name)*sizeof(char));
    if(classname == NULL)
    {
        PyErr_NoMemory();
        goto error;
    }
    
    strcpy((char*) classname, pyobj->ob_type->tp_name);
    jsclass->name = classname;
    
    jsclass->flags = flags;
    jsclass->addProperty = js_add_prop;
    jsclass->delProperty = js_del_prop;
    jsclass->getProperty = js_get_prop;
    jsclass->setProperty = js_set_prop;
    jsclass->enumerate = JS_EnumerateStub;
    jsclass->resolve = JS_ResolveStub;
    jsclass->convert = JS_ConvertStub;
    jsclass->finalize = js_finalize;
    jsclass->getObjectOps = NULL;
    jsclass->checkAccess = NULL;
    jsclass->call = js_call;
    jsclass->construct = js_ctor;
    jsclass->xdrObject = NULL;
    jsclass->hasInstance = NULL;
    jsclass->mark = NULL;
    jsclass->reserveSlots = NULL;
    
    curr = HashCObj_FromVoidPtr(jsclass);
    if(curr == NULL) goto error;
    if(Context_add_class(cx, pyobj->ob_type->tp_name, curr) < 0) goto error;

    ret = jsclass;
    goto success;

error:
    if(jsclass != NULL) free(jsclass);
    if(classname != NULL) free(classname);
success:
    return ret;
}

/*
    This is a fairly unsafe operation in so much as
    I'm relying on JavaScript to never call one of
    our callbacks on an object we didn't create.

    Also, of note, we're not incref'ing the Python
    object.
*/
PyObject*
get_py_obj(JSContext* cx, JSObject* obj)
{
    jsval priv;

    if(!JS_GetReservedSlot(cx, obj, 0, &priv))
    {
        js_error(cx, "Failed to get object's slot data.");
        return NULL;
    }

    return (PyObject*) JSVAL_TO_PRIVATE(priv);
}

PyObject*
mk_args_tpl(Context* pycx, JSContext* jscx, uintN argc, jsval* argv)
{
    PyObjectXDR tpl = PyTuple_New(argc);
    if(!tpl)
    {
        JS_ReportError(jscx, "Failed to build args value.");
        return NULL;
    }
    
    for(unsigned int idx = 0; idx < argc; idx++)
    {
        PyObject* tmp = js2py(pycx, argv[idx]);
        if(!tmp) return NULL;
        PyTuple_SET_ITEM(tpl.get(), idx, tmp);
    }

    return tpl.release();
}
