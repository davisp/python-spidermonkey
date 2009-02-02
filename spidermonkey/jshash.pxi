
def js_is_hash(Context cx, jsval v):
    return JSVAL_IS_OBJECT(v)

def py_is_hash(Context cx, object py_obj):
    return isinstance(py_obj, (types.DictionaryType, types.DictType))

cdef object js2py_hash(Context cx, jsval v):
    cdef JSObject* hash
    cdef JSObject *obj
    cdef JSIdArray *props
    cdef jsval jskey
    cdef jsval jsv
    cdef int i
    cdef dict ret
    cdef PyJSString js_str

    hash = JSVAL_TO_OBJECT(v)

    props = JS_Enumerate(cx.cx, hash)
    if props == NULL:
        raise JSError("Failed to enumerate hash properties.")

    ret = {}

    try:
        for i from 0 <= i < props.length:
            if not JS_IdToValue(cx.cx, (props.vector)[i], &jskey):
                raise JSError("Failed to convert dict to JavaScript.")
        
            if js_is_string(cx, jskey):
                key = js2py_string(cx, jskey)
                js_str = py2js_jsstring(cx.cx, key)
                
                if not JS_GetUCProperty(cx.cx, hash, js_str.chars(), js_str.length(), &jsv):
                    raise JSError("Faield to retrieve textual hash property.")
            elif js_is_int(cx, jskey):
                key = js2py_int(cx, jskey)
                if not JS_GetElement(cx.cx, hash, key, &jsv):
                    raise JSError("Failed to retrive numeric hash property.")
            else:
                raise AssertionError("Invalid JavaScript property.")

            ret[key] = js2py(cx, jsv)
    finally:
        JS_DestroyIdArray(cx.cx, props)

    return ret


cdef jsval py2js_hash(Context cx, dict py_obj, JSObject* parent):
    cdef JSObject* obj
    cdef jsval elem
    cdef PyJSString js_str
    
    obj = JS_NewObject(cx.cx, NULL, NULL, parent)
    if obj == NULL:
        raise JSError("Failed to create new JavaScript object for dict instance.")

    for k, v in py_obj.iteritems():
        js_str = py2js_jsstring(cx.cx, k)
        elem = py2js(cx, v, obj)
            
        if not JS_SetUCProperty(cx.cx, obj, js_str.chars(), js_str.length(), &elem):
            raise JSError("Failed to set JavaScript property for dict instance.")

    return OBJECT_TO_JSVAL(obj)
