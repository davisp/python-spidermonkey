
cdef jsval py2js(Context cx, object py_obj, JSObject* parent) except 0:
    if py_is_void(cx, py_obj):
        return py2js_void(cx, py_obj, parent)
    elif py_is_bool(cx, py_obj):
        return py2js_bool(cx, py_obj, parent)
    elif py_is_int(cx, py_obj):
        return py2js_int(cx, py_obj, parent)
    elif py_is_double(cx, py_obj):
        return py2js_double(cx, py_obj, parent)
    elif py_is_string(cx, py_obj):
        return py2js_string(cx, py_obj, parent)
    elif py_is_array(cx, py_obj):
        return py2js_array(cx, py_obj, parent)
    elif py_is_hash(cx, py_obj):
        return py2js_hash(cx, py_obj, parent)
    elif py_is_bound_method(cx, py_obj):
        return py2js_bound_method(cx, py_obj, parent)
    elif py_is_function(cx, py_obj):
        return py2js_function(cx, py_obj, parent)
    elif py_is_object(cx, py_obj):
        return py2js_object(cx, py_obj, parent)
    #elif py_is_class(cx, py_obj):
    #    return py2js_class(cx, py_obj, parent)
    else:
        raise TypeError("Unable to convert Python value to JavaScript: %r" % py_obj)

cdef object js2py(Context cx, jsval jsv):
    if js_is_void(cx, jsv):
        return js2py_void(cx, jsv)
    elif js_is_bool(cx, jsv):
        return js2py_bool(cx, jsv)
    elif js_is_int(cx, jsv):
        return js2py_int(cx, jsv)
    elif js_is_double(cx, jsv):
        return js2py_double(cx, jsv)
    elif js_is_string(cx, jsv):
        return js2py_string(cx, jsv)
    elif js_is_array(cx, jsv):
        return js2py_array(cx, jsv)
    elif js_is_function(cx, jsv): 
        return js2py_function(cx, jsv)
    elif js_is_object(cx, jsv):
        return js2py_object(cx, jsv)
    elif js_is_hash(cx, jsv):
        return js2py_hash(cx, jsv)
    else:
        raise TypeError("Unable to convert JavaScript value to Python: %r" % jsv)

cdef class Value:
    cdef jsval jsv

cdef Value js_create_value(jsval jsv):
    cdef Value v
    v = Value()
    v.jsv = jsv
    return v
