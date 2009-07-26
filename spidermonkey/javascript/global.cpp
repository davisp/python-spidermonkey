
#include <spidermonkey.h>
#include <jsobj.h>

static JSBool add_prop(JSContext*, JSObject*, jsval, jsval*);
static JSBool del_prop(JSContext*, JSObject*, jsval, jsval*);
static JSBool get_prop(JSContext*, JSObject*, jsval, jsval*);
static JSBool set_prop(JSContext*, JSObject*, jsval, jsval*);
static JSBool resolve(JSContext*, JSObject*, jsval);

JSClass
js_global_class = {
    "JSGlobalObjectClass",
    JSCLASS_GLOBAL_FLAGS,
    add_prop,
    del_prop,
    get_prop,
    set_prop,
    JS_EnumerateStub,
    resolve,
    JS_ConvertStub,
    JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

JSBool
add_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    JSObject* obj = NULL;

    if(JSVAL_IS_NULL(*rval) || !JSVAL_IS_OBJECT(*rval)) return JS_TRUE;

    obj = JSVAL_TO_OBJECT(*rval);
    if(JS_ObjectIsFunction(jscx, obj)) return set_prop(jscx, jsobj, key, rval);
    return JS_TRUE;
}

JSBool
del_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    Context* pycx = NULL;
    PyObject* pykey = NULL;
    PyObject* pyval = NULL;
    JSBool ret = JS_FALSE;

    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get Python context.");
        goto done;
    }

    // Bail if there's no registered global handler.
    if(pycx->pyglobal == NULL)
    {
        ret = JS_TRUE;
        goto done;
    }
    
    // Check access to python land.
    if(Context_has_access(pycx, jscx, pycx->pyglobal, pykey) <= 0) goto done;

    // Bail if the global doesn't have a __delitem__
    if(!PyObject_HasAttrString(pycx->pyglobal, "__delitem__"))
    {
        ret = JS_TRUE;
        goto done;
    }

    pykey = js2py(pycx, key);
    if(pykey == NULL) goto done;

    if(PyObject_DelItem(pycx->pyglobal, pykey) < 0) goto done;

    ret = JS_TRUE;

done:
    Py_XDECREF(pykey);
    Py_XDECREF(pyval);
    return ret;
}

JSBool
get_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    Context* pycx = NULL;
    PyObject* pykey = NULL;
    PyObject* pyval = NULL;
    JSBool ret = JS_FALSE;

    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get Python context.");
        goto done;
    }

    // Bail if there's no registered global handler.
    if(pycx->pyglobal == NULL)
    {
        ret = JS_TRUE;
        goto done;
    }

    pykey = js2py(pycx, key);
    if(pykey == NULL) goto done;

    if(Context_has_access(pycx, jscx, pycx->pyglobal, pykey) <= 0) goto done;

    pyval = PyObject_GetItem(pycx->pyglobal, pykey);
    if(pyval == NULL)
    {
        if(PyErr_GivenExceptionMatches(PyErr_Occurred(), PyExc_KeyError))
        {
            PyErr_Clear();
            ret = JS_TRUE;
        }
        goto done;
    }

    *rval = py2js(pycx, pyval);
    if(*rval == JSVAL_VOID) goto done;
    ret = JS_TRUE;

done:
    Py_XDECREF(pykey);
    Py_XDECREF(pyval);
    return ret;
}

JSBool
set_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    Context* pycx = NULL;
    PyObject* pykey = NULL;
    PyObject* pyval = NULL;
    JSBool ret = JS_FALSE;

    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get Python context.");
        goto done;
    }

    // Bail if there's no registered global handler.
    if(pycx->pyglobal == NULL)
    {
        ret = JS_TRUE;
        goto done;
    }

    pykey = js2py(pycx, key);
    if(pykey == NULL) goto done;

    if(Context_has_access(pycx, jscx, pycx->pyglobal, pykey) <= 0) goto done;

    pyval = js2py(pycx, *rval);
    if(pyval == NULL) goto done;

    if(PyObject_SetItem(pycx->pyglobal, pykey, pyval) < 0) goto done;

    ret = JS_TRUE;

done:
    Py_XDECREF(pykey);
    Py_XDECREF(pyval);
    return ret;
}

JSBool
resolve(JSContext* jscx, JSObject* jsobj, jsval key)
{
    Context* pycx = NULL;
    PyObject* pykey = NULL;
    jsid pid;
    JSBool ret = JS_FALSE;

    pycx = (Context*) JS_GetContextPrivate(jscx);
    if(pycx == NULL)
    {
        JS_ReportError(jscx, "Failed to get Python context.");
        goto done;
    }

    // Bail if there's no registered global handler.
    if(pycx->pyglobal == NULL)
    {
        ret = JS_TRUE;
        goto done;
    }

    pykey = js2py(pycx, key);
    if(pykey == NULL) goto done;
    
    if(Context_has_access(pycx, jscx, pycx->pyglobal, pykey) <= 0) goto done;

    if(!PyMapping_HasKey(pycx->pyglobal, pykey))
    {
        ret = JS_TRUE;
        goto done;
    }

    if(!JS_ValueToId(jscx, key, &pid))
    {
        JS_ReportError(jscx, "Failed to convert property id.");
        goto done;
    }

    if(!js_DefineProperty(jscx, pycx->jsglobal, pid, JSVAL_VOID, NULL, NULL,
                            JSPROP_SHARED, NULL))
    {
        JS_ReportError(jscx, "Failed to define property.");
        goto done;
    }

    ret = JS_TRUE;

done:
    Py_XDECREF(pykey);
    return ret;
}
