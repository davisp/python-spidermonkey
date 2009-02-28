
def js_is_int(Context cx, jsval jsv):
    return JSVAL_IS_INT(jsv)

def py_is_int(Context cx, object py_obj):
    return isinstance(py_obj, (types.IntType, types.LongType))

cdef object js2py_int(Context cx, jsval jsv):
    return JSVAL_TO_INT(jsv)

cdef jsval py2js_int(Context cx, jsint py_obj, JSObject* parent):
    return INT_TO_JSVAL(py_obj)

