


def js_is_string(Context cx, jsval jsv):
    return JSVAL_IS_STRING(jsv)

def py_is_string(Context cx, object py_obj):
    return isinstance(py_obj, types.UnicodeType)

cdef jsval py2js_string(Context cx, object py_obj, JSObject* parent):
    cdef JSString* conv
    conv = py2js_jsstring(cx.cx, py_obj)
    return STRING_TO_JSVAL(conv)

cdef object js2py_string(Context cx, jsval jsv):
    return js2py_jsstring(JSVAL_TO_STRING(jsv))
