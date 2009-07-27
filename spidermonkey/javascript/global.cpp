
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
    if(JSVAL_IS_NULL(*rval) || !JSVAL_IS_OBJECT(*rval))
        return JS_TRUE;

    if(JS_ObjectIsFunction(jscx, JSVAL_TO_OBJECT(*rval)))
        return set_prop(jscx, jsobj, key, rval);

    return JS_TRUE;
}

JSBool
del_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    PyPtr<Context> pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");

    if(pycx->pyglobal == NULL) return JS_TRUE;
    if(!PyObject_HasAttrString(pycx->pyglobal, "__delitem__")) return JS_TRUE;

    PyObjectXDR pykey = js2py(pycx.get(), key);
    if(!pykey) return js_error(jscx, "Failed to covert key.");

    if(Context_has_access(pycx.get(), jscx, pycx->pyglobal, pykey.get()) <= 0)
        return js_error(jscx, "Access denied.");
    
    if(PyObject_DelItem(pycx->pyglobal, pykey.get()) < 0)
        return js_error(jscx, "Failed to delete key.");
    
    return JS_TRUE;
}

JSBool
get_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    PyPtr<Context> pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");
    
    if(pycx->pyglobal == NULL) return JS_TRUE;
    
    PyObjectXDR pykey = js2py(pycx.get(), key);
    if(!pykey) return js_error(jscx, "Failed to convert key.");
    
    if(Context_has_access(pycx.get(), jscx, pycx->pyglobal, pykey.get()) <= 0)
        return js_error(jscx, "Access denied.");

    PyObjectXDR pyval = PyObject_GetItem(pycx->pyglobal, pykey.get());
    if(!pyval)
    {
        if(PyErr_GivenExceptionMatches(PyErr_Occurred(), PyExc_KeyError))
        {
            PyErr_Clear();
            return JS_TRUE;
        }
        return js_error(jscx, "Failed to get value.");
    }

    *rval = py2js(pycx.get(), pyval.get());
    if(*rval == JSVAL_VOID)
        return js_error(jscx, "Failed to convert value.");
    
    return JS_TRUE;
}

JSBool
set_prop(JSContext* jscx, JSObject* jsobj, jsval key, jsval* rval)
{
    PyPtr<Context> pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");
    
    if(pycx->pyglobal == NULL) return JS_TRUE;

    PyObjectXDR pykey = js2py(pycx.get(), key);
    if(!pykey) return js_error(jscx, "Failed to convert key.");

    if(Context_has_access(pycx.get(), jscx, pycx->pyglobal, pykey.get()) <= 0)
        return js_error(jscx, "Access denied.");

    PyObjectXDR pyval = js2py(pycx.get(), *rval);
    if(!pyval) return js_error(jscx, "Failed to convert value.");

    if(PyObject_SetItem(pycx->pyglobal, pykey.get(), pyval.get()) < 0)
        return js_error(jscx, "Failed to set value.");

    return JS_TRUE;
}

JSBool
resolve(JSContext* jscx, JSObject* jsobj, jsval key)
{
    jsid pid;

    PyPtr<Context> pycx = (Context*) JS_GetContextPrivate(jscx);
    if(!pycx) return js_error(jscx, "Failed to get Python context.");

    if(pycx->pyglobal == NULL) return JS_TRUE;
    
    PyObjectXDR pykey = js2py(pycx.get(), key);
    if(!pykey) return js_error(jscx, "Failed to convert key.");
    
    if(Context_has_access(pycx.get(), jscx, pycx->pyglobal, pykey.get()) <= 0)
        return js_error(jscx, "Access denied.");

    if(!PyMapping_HasKey(pycx->pyglobal, pykey.get())) return JS_TRUE;

    if(!JS_ValueToId(jscx, key, &pid))
        return js_error(jscx, "Failed to convert property id.");

    if(!js_DefineProperty(jscx, pycx->jsglobal, pid, JSVAL_VOID, NULL, NULL,
                            JSPROP_SHARED, NULL))
        return js_error(jscx, "Failed to define property.");

    return JS_TRUE;
}
