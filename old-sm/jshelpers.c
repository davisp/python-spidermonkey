JSString* py2js_jsstring_c(JSContext* cx, PyObject* str)
{
    PyObject* encoded = NULL;
    char* bytes;
    Py_ssize_t len;
    
    if(!PyUnicode_Check(str))
    {
        return NULL;
    }
        
    encoded = PyUnicode_AsEncodedString(str, "utf-16", "strict");
    if(encoded == NULL)
    {
        return NULL;
    }
        
    if(PyString_AsStringAndSize(encoded, &bytes, &len) < 0)
    {
        return NULL;
    }

    if(len < 4)
    {
        return NULL;
    }

    // No idea why python adds FFFE to encoded UTF-16 data.
    return JS_NewUCStringCopyN(cx, bytes+2, (len/2)-1);
}

PyObject* js2py_jsstring_c(JSString* str)
{
    jschar* bytes;
    size_t len;
        
    if(str == NULL)
    {
        return NULL;
    }
    
    len = JS_GetStringLength(str);
    bytes = JS_GetStringChars(str);

    return PyUnicode_Decode((const char*) bytes, (size_t) (len * 2), "utf-16", "strict");
}

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
    JSClass* class = JS_GetClass(cx, js_obj);
    
    if(class == NULL)
    {
        return JS_FALSE;
    }
    
    if(!(class->flags & JSCLASS_HAS_PRIVATE))
    {
        return JS_FALSE;
    }
    
    if(JS_GetPrivate(cx, js_obj) == NULL)
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

JSObject *
js_make_global_object(JSContext *cx)
{
    return JS_NewObject(cx, &js_global_class, NULL, NULL);
}
