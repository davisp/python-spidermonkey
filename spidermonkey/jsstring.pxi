


def js_is_string(Context cx, jsval jsv):
    return JSVAL_IS_STRING(jsv)

def py_is_string(Context cx, object py_obj):
    return isinstance(py_obj, types.StringTypes)

cdef object js2py_string(Context cx, jsval jsv):
    cdef JSString* s
    s = JSVAL_TO_STRING(jsv)
    return JS_GetStringBytes(s)

#################################################
# NEEED TO REWWRITE TO USE JS_NewExternalString #
#################################################
cdef jsval py2js_string(Context cx, object py_obj, JSObject* parent):
    cdef JSString* s
    s = JS_NewStringCopyN(cx.cx, py_obj, len(py_obj))
    return STRING_TO_JSVAL(s)
