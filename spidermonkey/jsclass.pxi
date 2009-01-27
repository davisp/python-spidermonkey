
cdef JSBool __constructor_callback__(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval):
    cdef Context pycx
    cdef ClassAdapter adapter
    cdef ObjectAdapter objadapter
    cdef JSObject* instobj
    cdef int i
    
    try:
        pycx = js_context_fetch(cx)
        adapter = js_object_fetch(obj)
        
        args = []
        for i from 0 <= i < argc:
            args.append(js2py(pycx, argv[i]))

        if hasattr(adaptor, "__jsinit__"):
            py_rval = adaptor.py_class.__jsinit__(pycx, *args)
        else:
            py_rval = adaptor.py_class(pycx, *args)

        rval[0] = py2js(pycx, py_rval, NULL)

        # Register after conversion so we don't keep it around
        # if conversion fails.
        if JSVAL_IS_OBJECT(rval[0]):
            objadapter = ObjectAdapter(pycx, adapter, None, py_rval)
            instobj = JSVAL_TO_OBJECT(rval[0])
            js_object_attach(instobj, <PyObject*> py_rval)
            objadapter.js_obj = instobj
        pycx.register(py_rval)
        
        return JS_TRUE
    except:
        return report_python_error(cx)

cdef JSBool __resolve_global_callback__(JSContext* cx, JSObject* js_obj, jsval jsv):
    cdef Context pycx
    cdef ObjectAdapter adapter
    cdef object py_obj
    cdef object key
    cdef int i

    try:
        pycx = js_context_fetch(cx)
        adapter = js_object_fetch(js_obj)
        py_obj = adapter.obj
        key = js2py(pycx, jsv)
        
        if isinstance(key, types.StringTypes) and hasattr(py_obj, key):
            # Bind to root object.
            # Will ref the obj so it doesn't get discarded.
            pycx.bind(key, getattr(py_obj, key))

        return JS_TRUE
    except:
        return report_python_error(cx)

cdef JSBool __get_property_callback__(JSContext* cx, JSObject* js_obj, jsval jsv, jsval* rval):
    cdef Context pycx
    cdef ObjectAdapter adapter
    cdef object py_obj
    cdef object key
    cdef object attr

    try:
        pycx = js_context_fetch(cx)
        adapter = js_object_fetch(js_obj)
        py_obj = adapter.obj
        key = js2py(pycx, jsv)
        
        if isinstance(key, (types.IntType, types.LongType)):
            try:
                attr = getattr(py_obj, key)
                rval[0] = py2js(pycx, attr, NULL)
                pycx.register_py(attr)
            except:
                rval[0] = JS_VOID
        elif isinstance(key, types.StringTypes) and hasattr(py_obj, key):
            try:
                attr = getattr(py_obj, key)
                rval[0] = py2js(pycx, attr, NULL)
                pycx.register_py(attr)
            except:
                rval[0] = JS_VOID
        elif key is None:
            rval[0] = JS_VOID
        else:
            raise AssertionError("Invalid key: %r" % key)
            
        return JS_TRUE
    except:
        return report_python_error(cx)

cdef JSBool __set_property_callback__(JSContext* cx, JSObject* js_obj, jsval jsv, jsval rval[0]):
    cdef Context pycx
    cdef ObjectAdapter adapter
    cdef object py_obj
    cdef object key
    cdef object val

    try:
        pycx = js_context_fetch(cx)
        adapter = js_object_fetch(js_obj)
        py_obj = adapter.py_obj
        key = js2py(pycx, jsv)
        value = js2py(pycx, rval[0])

        if isinstance(key, (types.IntType, types.LongType)):
            py_obj[key] = value
        elif isinstance(key, types.StringTypes) and hasattr(py_obj, key):
            attr = getattr(py_obj, key)
            if not callable(attr):
                setattr(py_obj, key, value)
        else:
            raise AssertionError("Invalid key: %r" % key)

        return JS_TRUE
    except:
        return report_python_error(cx)


cdef void __finalize_callback__(JSContext* cx, JSObject* js_obj):
    cdef Context pycx
    cdef ObjectAdapter py_obj

    try:
        pycx = js_context_fetch(cx)
        py_obj = js_object_destroy(js_obj)
    except:
        report_python_error(cx)

def js_classname(obj):
    if inspect.isclass(obj):
        return obj.__name__
    else:
        return obj.__class__.__name__

cdef class ClassAdapter:
    cdef Context cx
    cdef ObjectAdapter parent
    cdef JSClass* js_class
    cdef object py_class

    def __cinit__(ClassAdapter self, Context cx, ObjectAdapter parent, py_class, bind_constructur, is_global, flags):
        cdef JSObject* obj
        
        self.cx = cx
        self.parent = parent
        self.py_class = py_class
        
        name = js_classname(py_class)

        self.js_class = <JSClass*> xmalloc(sizeof(JSClass))
        self.js_class.name = <char*> xmalloc((len(name) + 1) * sizeof(char))
        strcpy(self.js_class.name, name)

        self.js_class.flags = flags
        self.js_class.addProperty = JS_PropertyStub
        self.js_class.delProperty = JS_PropertyStub
        self.js_class.getProperty = __get_property_callback__
        self.js_class.setProperty = __set_property_callback__
        self.js_class.enumerate = JS_EnumerateStub
        self.js_class.convert = JS_ConvertStub
        self.js_class.finalize = __finalize_callback__
        self.js_class.getObjectOps = NULL
        self.js_class.checkAccess = NULL
        self.js_class.call = NULL
        self.js_class.construct = NULL
        self.js_class.xdrObject = NULL
        self.js_class.hasInstance = NULL
        self.js_class.mark = NULL
        self.js_class.reserveSlots = NULL
        
        if is_global:
            self.js_class.resolve = __resolve_global_callback__
        else:
            self.js_class.resolve = JS_ResolveStub
        
        if bind_constructor:
            if JS_InitClass(self.cx.cx, parent.js_obj, NULL, self.js_class,
                            __constructor_callback__, 0, NULL, NULL, NULL, NULL) == NULL: 
                raise JSError("Failed to bind Python adapter class.")

        js_object_attach(obj, <PyObject*> self)

    def __dealloc__(self):
        free(self.js_class.name)
        free(self.js_class)

    def as_value(Context self):
        cdef JSObject* obj
        obj = JS_GetClassObject(self.js_class)
        return js_create_value(OBJECT_TO_JSVAL(obj))










