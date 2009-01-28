void
js_context_attach(JSContext* cx, PyObject* obj)
{
    Py_XINCREF(obj);
    JS_SetContextPrivate(cx, (void*) obj);
}

JSBool
js_context_has_data(JSContext* cx)
{
    if(JS_GetContextPrivate(cx) == NULL)
    {
        return JS_FALSE;
    }
    
    return JS_TRUE;
}

PyObject* 
js_context_fetch(JSContext* cx)
{
    PyObject* obj = (PyObject*) JS_GetContextPrivate(cx);
    Py_XINCREF(obj);
    return obj;
}

PyObject*
js_context_destroy(JSContext* cx)
{
    PyObject* ret = (PyObject*) JS_GetContextPrivate(cx);
    return ret;
}

void
js_object_attach(JSContext* cx, JSObject* js_obj, PyObject* py_obj)
{   
    Py_XINCREF(py_obj);
    JS_SetPrivate(cx, js_obj, (void*) py_obj);
}

JSBool
js_object_has_data(JSContext* cx, JSObject* js_obj)
{
    void* data = JS_GetPrivate(cx, js_obj);
    if(data == NULL)
    {
        return JS_FALSE;
    }
    
    return JS_TRUE;
}

PyObject*
js_object_fetch(JSContext* cx, JSObject* js_obj)
{
    PyObject* py_obj = (PyObject*) JS_GetPrivate(cx, js_obj);
    Py_XINCREF(py_obj);
    return py_obj;
}

PyObject*
js_object_destroy(JSContext* cx, JSObject* js_obj)
{
    return (PyObject*) JS_GetPrivate(cx, js_obj);
}

void
js_function_attach(JSContext* cx, JSObject* js_obj, PyObject* py_obj)
{   
    Py_XINCREF(py_obj);
    jsval slot = PRIVATE_TO_JSVAL(py_obj);
    JS_SetReservedSlot(cx, js_obj, 0, slot);
}

JSBool
js_function_has_data(JSContext* cx, JSObject* js_obj)
{
    jsval slot;
    if(JS_GetReservedSlot(cx, js_obj, 0, &slot) != JS_TRUE)
    {
        return JS_FALSE;
    }
    
    void* data = JSVAL_TO_PRIVATE(slot);
    
    if(data == NULL)
    {
        return JS_FALSE;
    }
    
    return JS_TRUE;
}

PyObject*
js_function_fetch(JSContext* cx, JSObject* js_obj)
{
    jsval slot;
    if(JS_GetReservedSlot(cx, js_obj, 0, &slot) != JS_TRUE)
    {
        return JS_FALSE;
    }
    
    PyObject* py_obj = (PyObject*) JSVAL_TO_PRIVATE(slot);
    Py_XINCREF(py_obj);
    return py_obj;
}

PyObject*
js_function_destroy(JSContext* cx, JSObject* js_obj)
{
    jsval slot;
    if(JS_GetReservedSlot(cx, js_obj, 0, &slot) != JS_TRUE)
    {
        return JS_FALSE;
    }
    
    PyObject* py_obj = (PyObject*) JSVAL_TO_PRIVATE(slot);
    return py_obj;
}

static JSClass js_global_class =
{
    "RootObjectClass",
    0,
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

JSObject *
js_make_global_object(JSContext *cx)
{
    return JS_NewObject(cx, &js_global_class, 0, 0);
}
