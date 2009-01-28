
cdef class ObjectAdapter:
    cdef Context cx
    cdef ClassAdapter cl_adapter
    cdef ObjectAdapter parent
    cdef JSObject* js_obj
    cdef object py_obj

    def __cinit__(ObjectAdapter self, Context cx, ClassAdapter adapter, ObjectAdapter parent, object obj):
        self.cx = cx
        self.cl_adapter = adapter
        self.parent = parent
        self.js_obj = NULL
        self.py_obj = obj

    def __repr__(self):
        return "<spidermonkey.ObjectAdapter: %r>" % self.py_obj

def js_is_object(Context cx, jsval jsv):
    if not JSVAL_IS_OBJECT(jsv):
        return False
    if not js_object_has_data(cx.cx, JSVAL_TO_OBJECT(jsv)):
        return False
    return True

def py_is_object(Context cx, object py_obj):
    return not isinstance(py_obj, types.TypeType)

cdef object js2py_object(Context cx, jsval jsv):
    cdef ClassAdapter ca
    cdef ObjectAdapter oa
    cdef FunctionAdapter fa
    
    tmp = js_object_fetch(cx.cx, JSVAL_TO_OBJECT(jsv))
    if isinstance(tmp, ClassAdapter):
        ca = tmp
        return ca.py_obj
    elif isinstance(tmp, ObjectAdapter):
        oa = tmp
        return oa.py_obj
    elif isinstance(tmp, FunctionAdapter):
        fa = tmp
        return fa.py_obj
    else:
        raise JSError("Failed to unwrap Python object.")

cdef jsval py2js_object(Context cx, object py_obj, JSObject* parent):
    cdef ClassAdapter ca
    cdef ObjectAdapter oa
    cdef JSObject* js_obj
    cdef jsval rval

    ca = cx.install_class(py_obj.__class__)
    js_obj = JS_NewObject(cx.cx, ca.js_class, NULL, NULL)
    if js_obj == NULL:
        raise JSError("Failed to create object.")
    oa = ObjectAdapter(cx, ca, None, py_obj)
    oa.js_obj = js_obj
    js_object_attach(cx.cx, oa.js_obj, <PyObject*> oa)

    return OBJECT_TO_JSVAL(js_obj)
