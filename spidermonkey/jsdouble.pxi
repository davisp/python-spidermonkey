
def js_is_double(Context cx, jsval jsv):
    return JSVAL_IS_DOUBLE(jsv)

def py_is_double(Context cx, object py_obj):
    return isinstance(py_obj, types.FloatType)

cdef object js2py_double(Context cx, jsval jsv):
    return JSVAL_TO_DOUBLE(jsv)[0]

cdef jsval py2js_double(Context cx, double py_obj, JSObject* parent):
    cdef jsval ret
    if not JS_NewNumberValue(cx.cx, py_obj, &ret):
        raise JSError("Failed to convert double.")
    return ret
