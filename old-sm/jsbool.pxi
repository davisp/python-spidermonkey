def js_is_bool(Context cx, jsval jsv):
    return JSVAL_IS_BOOLEAN(jsv)

def py_is_bool(Context cx, object py_obj):
    return isinstance(py_obj, types.BooleanType)

cdef object js2py_bool(Context cx, jsval jsv):
    if JSVAL_TO_BOOLEAN(jsv):
        return True
    else:
        return False

cdef jsval py2js_bool(Context cx, object py_obj, JSObject* parent):
    if py_obj:
        return BOOLEAN_TO_JSVAL(JS_TRUE)
    else:
        return BOOLEAN_TO_JSVAL(JS_FALSE)
