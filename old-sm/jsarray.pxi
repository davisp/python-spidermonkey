
def js_is_array(Context cx, jsval v):
    cdef JSObject* obj
    if not JSVAL_IS_OBJECT(v):
        return False
    obj = JSVAL_TO_OBJECT(v)
    return JS_IsArrayObject(cx.cx, obj)

def py_is_array(Context cx, object py_obj):
    return isinstance(py_obj, (types.ListType, types.TupleType))

cdef object js2py_array(Context cx, jsval v):
    cdef jsuint nr_elems
    cdef jsval elem
    cdef JSObject *jsobj
    cdef int i
    cdef list ret
    
    jsobj = JSVAL_TO_OBJECT(v)
    JS_GetArrayLength(cx.cx, jsobj, &nr_elems)
    ret = [None] * nr_elems

    for i from 0 <= i < nr_elems:
        if not JS_GetElement(cx.cx, jsobj, i, &elem):
            raise JSError("Failed to convert JavaScript array to Python.")
        ret[i] = js2py(cx, elem)

    return ret

cdef jsval py2js_array(Context cx, list py_obj, JSObject* parent):
    cdef int nr_elems, i
    cdef JSObject* ret
    cdef jsval elem

    nr_elems = len(py_obj)
    ret = JS_NewArrayObject(cx.cx, 0, NULL)
    
    for i from 0 <= i < nr_elems:
        elem = py2js(cx, py_obj[i], parent)
        if not JS_SetElement(cx.cx, ret, i, &elem):
            raise JSError("Failed to convert Python sequence type to JavaScript.")
    
    return OBJECT_TO_JSVAL(ret)