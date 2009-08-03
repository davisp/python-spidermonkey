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
    Context* pycx = NULL;
    PyObject* pyobj = NULL;
    PyObject* pykey = NULL;
    PyObject* pyval = NULL;
    JSBool ret = JS_FALSE;
    
    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to find a Python Context.");
        goto error;
    }
    
    pyobj = get_py_obj(jscx, jsobj);
    if(pyobj == NULL)
    {
        JS_ReportError(jscx, "Failed to find a Python object.");
        goto error;
    }
    
    pykey = js2py(pycx, key);
    if(pykey == NULL)
    {
        JS_ReportError(jscx, "Failed to convert key to Python.");
        goto error;
    }

    if(Context_has_access(pycx, jscx, pyobj, pykey) <= 0) goto error;

    pyval = js2py(pycx, *val);
    if(pyval == NULL)
    {
        JS_ReportError(jscx, "Failed to convert value to Python.");
        goto error;
    }

    if(PyObject_SetItem(pyobj, pykey, pyval) < 0)
    {
        PyErr_Clear();
        if(PyObject_SetAttr(pyobj, pykey, pyval) < 0) goto error;
    }

    ret = JS_TRUE;
    goto success;
    
error:
success:
    Py_XDECREF(pykey);
    Py_XDECREF(pyval);
    return ret;
}

void
js_finalize(JSContext* jscx, JSObject* jsobj)
{
    Context* pycx = (Context*) JS_GetContextPrivate(jscx);
    PyObject* pyobj = NULL;
  
    if(pycx == NULL)
    {
        // Not much else we can do but yell.
        fprintf(stderr, "*** NO PYTHON CONTEXT ***\n");
        return;
    }

    JS_BeginRequest(jscx);
    pyobj = get_py_obj(jscx, jsobj);
    JS_EndRequest(jscx);

    Py_DECREF(pyobj);
}

JSBool
js_call(JSContext* jscx, JSObject* jsobj, uintN argc, jsval* argv, jsval* rval)
{
    Context* pycx = NULL;
    PyObject* pyobj = NULL;
    PyObject* tpl = NULL;
    PyObject* ret = NULL;
    PyObject* attrcheck = NULL;
    JSBool jsret = JS_FALSE;
    
    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get Python context.");
        goto error;
    }
    
    pyobj = get_py_obj(jscx, JSVAL_TO_OBJECT(argv[-2]));
    
    if(!PyCallable_Check(pyobj))
    {
        JS_ReportError(jscx, "Object is not callable.");
        goto error;
    }

    // Use '__call__' as a notice that we want to execute a function.
    attrcheck = PyString_FromString("__call__");
    if(attrcheck == NULL) goto error;

    if(Context_has_access(pycx, jscx, pyobj, attrcheck) <= 0) goto error;

    tpl = mk_args_tpl(pycx, jscx, argc, argv);
    if(tpl == NULL) goto error;
    
    ret = PyObject_Call(pyobj, tpl, NULL);
    if(ret == NULL)
    {
        if(!PyErr_Occurred())
        {
            PyErr_SetString(PyExc_RuntimeError, "Failed to call object.");
        }
        JS_ReportError(jscx, "Failed to call object.");
        goto error;
    }
    
    *rval = py2js(pycx, ret);
    if(*rval == JSVAL_VOID)
    {
        JS_ReportError(jscx, "Failed to convert Python return value.");
        goto error;
    }

    jsret = JS_TRUE;
    goto success;

error:
success:
    Py_XDECREF(tpl);
    Py_XDECREF(ret);
    Py_XDECREF(attrcheck);
    return jsret;
}

JSBool
js_ctor(JSContext* jscx, JSObject* jsobj, uintN argc, jsval* argv, jsval* rval)
{
    Context* pycx = NULL;
    PyObject* pyobj = NULL;
    PyObject* tpl = NULL;
    PyObject* ret = NULL;
    PyObject* attrcheck = NULL;
    JSBool jsret = JS_FALSE;
    
    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get Python context.");
        goto error;
    }
    
    pyobj = get_py_obj(jscx, JSVAL_TO_OBJECT(argv[-2]));
    
    if(!PyCallable_Check(pyobj))
    {
        JS_ReportError(jscx, "Object is not callable.");
        goto error;
    }

    if(!PyType_Check(pyobj))
    {
        PyErr_SetString(PyExc_TypeError, "Object is not a Type object.");
        goto error;
    }

    // Use '__init__' to signal use as a constructor.
    attrcheck = PyString_FromString("__init__");
    if(attrcheck == NULL) goto error;

    if(Context_has_access(pycx, jscx, pyobj, attrcheck) <= 0) goto error;

    tpl = mk_args_tpl(pycx, jscx, argc, argv);
    if(tpl == NULL) goto error;
    
    ret = PyObject_CallObject(pyobj, tpl);
    if(ret == NULL)
    {
        JS_ReportError(jscx, "Failed to construct object.");
        goto error;
    }
    
    *rval = py2js(pycx, ret);
    if(*rval == JSVAL_VOID)
    {
        JS_ReportError(jscx, "Failed to convert Python return value.");
        goto error;
    }

    jsret = JS_TRUE;
    goto success;

error:
success:
    Py_XDECREF(tpl);
    Py_XDECREF(ret);
    return jsret;
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
    PyObject* tpl = NULL;
    PyObject* tmp = NULL;
    
    tpl = PyTuple_New(argc);
    if(tpl == NULL)
    {
        JS_ReportError(jscx, "Failed to build args value.");
        goto error;
    }
    
    for(unsigned int idx = 0; idx < argc; idx++)
    {
        tmp = js2py(pycx, argv[idx]);
        if(tmp == NULL) goto error;
        PyTuple_SET_ITEM(tpl, idx, tmp);
    }

    goto success;

error:
    Py_XDECREF(tpl);
success:
    return tpl;
}
