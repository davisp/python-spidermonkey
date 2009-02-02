


def js_is_string(Context cx, jsval jsv):
    return JSVAL_IS_STRING(jsv)

def py_is_string(Context cx, object py_obj):
    if isinstance(py_obj, types.StringTypes):
        if not isinstance(py_obj, types.UnicodeType):
            raise UnicodeError("Non unicode strings not allowed in spidermonkey.")
        return True
    return False

cdef jsval py2js_string(Context cx, object py_obj, JSObject* parent):
    cdef PyJSString js_str
    js_str = py2js_jsstring(cx.cx, py_obj)
    return STRING_TO_JSVAL(js_str.data)

cdef object js2py_string(Context cx, jsval jsv):
    return js2py_jsstring(JSVAL_TO_STRING(jsv))
