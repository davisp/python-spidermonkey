void
js_context_attach(JSContext* cx, PyObject* obj)
{
    Py_INCREF(obj);
    JS_SetContextPrivate(cx, (void*) obj);
}

PyObject* 
js_context_fetch(JSContext* cx)
{
    PyObject* obj = (PyObject*) JS_GetContextPrivate(cx);
    Py_INCREF(obj);
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
    Py_INCREF(py_obj);
    JS_SetPrivate(cx, js_obj, (void*) py_obj);
}

PyObject*
js_object_fetch(JSContext* cx, JSObject* js_obj)
{
    PyObject* py_obj = (PyObject*) JS_GetPrivate(cx, js_obj);
    Py_INCREF(py_obj);
    return py_obj;
}

PyObject*
js_object_destroy(JSContext* cx, JSObject* js_obj)
{
    return (PyObject*) JS_GetPrivate(cx, js_obj);
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
    return JS_NewObject(cx, &global_class, 0, 0);
}
