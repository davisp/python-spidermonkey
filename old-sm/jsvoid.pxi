
def js_is_void(Context cx, jsval jsv):
    return JSVAL_IS_VOID(jsv) or jsv == 0

def py_is_void(Context cx, object py_obj):
    return isinstance(py_obj, (types.NoneType))

cdef object js2py_void(Context cx, jsval jsv):
    return None

cdef jsval py2js_void(Context cx, object py_obj, JSObject* parent):
    return JSVAL_VOID
