
cdef JSBool __constructor_callback__(JSContext* cx, JSObject* js_obj, uintN argc, jsval* argv, jsval* rval):
    cdef Context pycx
    cdef ClassAdapter adapter
    cdef ObjectAdapter oa
    cdef JSObject* proto
    cdef int i
    
    try:
        if not js_context_has_data(cx):
            raise JSError("Unknown JSContext object.")
        
        proto = JS_GetPrototype(cx, js_obj)
        if proto == NULL:
            raise JSError("Object has no prototype.")
        
        if not js_object_has_data(cx, proto):
            raise JSError("Unknown constructor callback.")
        
        pycx = js_context_fetch(cx)
        adapter = js_object_fetch(cx, proto)
        
        args = []
        for i from 0 <= i < argc:
            args.append(js2py(pycx, argv[i]))

        if hasattr(adapter.py_class, unicode("__jsinit__")):
            py_rval = adapter.py_class.__jsinit__(pycx, *args)
        else:
            py_rval = adapter.py_class(*args)

        rval[0] = py2js(pycx, py_rval, NULL)

        # Register after conversion so we don't keep it around
        # if conversion fails.
        if JSVAL_IS_OBJECT(rval[0]):
            oa = ObjectAdapter(pycx, adapter, None, py_rval)
            oa.js_obj = js_obj
            js_object_attach(cx, js_obj, <PyObject*> oa)
        
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
        if not js_context_has_data(cx):
            raise JSError("Unknown JSContext object.")

        pycx = js_context_fetch(cx)
        key = js2py(pycx, jsv)

        if not js_object_has_data(cx, js_obj):
            return JS_TRUE;
                
        adapter = js_object_fetch(cx, js_obj)
        py_obj = adapter.py_obj
                
        if isinstance(key, types.UnicodeType) and hasattr(py_obj, key):
            pycx.bind(key, getattr(py_obj, key))

        return JS_TRUE
    except:
        traceback.print_exc()
        return report_python_error(cx)

cdef JSBool __get_property_callback__(JSContext* cx, JSObject* js_obj, jsval jsv, jsval* rval):
    cdef Context pycx
    cdef ObjectAdapter adapter
    cdef object py_obj
    cdef object key
    cdef object attr


    try:
        if not js_context_has_data(cx):
            raise JSError("Unknown JSContext object.")

        pycx = js_context_fetch(cx)
        key = js2py(pycx, jsv)
                
        if not js_object_has_data(cx, js_obj):
            return JS_TRUE
        
        adapter = js_object_fetch(cx, js_obj)
        py_obj = adapter.py_obj
        
        if isinstance(key, (types.IntType, types.LongType)):
            try:
                attr = py_obj[key]
            except:
                pass
            else:
                rval[0] = py2js(pycx, attr, NULL)
        elif isinstance(key, types.UnicodeType):
            if key[:1] != unicode("_"):
                try:
                    attr = getattr(py_obj, key)
                except:
                    pass
                else:
                    rval[0] = py2js(pycx, attr, js_obj)
        elif key is None:
            rval[0] = JSVAL_VOID
        else:
            raise AssertionError("Invalid key: %r" % key)
            
        return JS_TRUE
    except:
        traceback.print_exc()
        return report_python_error(cx)

cdef JSBool __set_property_callback__(JSContext* cx, JSObject* js_obj, jsval jsv, jsval rval[0]):
    cdef Context pycx
    cdef ObjectAdapter adapter
    cdef object py_obj
    cdef object key
    cdef object val

    try:
        if not js_context_has_data(cx):
            raise JSError("Unknown JSContext object.")

        pycx = js_context_fetch(cx)
        key = js2py(pycx, jsv)
        value = js2py(pycx, rval[0])
                
        if not js_object_has_data(cx, js_obj):
            return JS_TRUE
        
        adapter = js_object_fetch(cx, js_obj)
        py_obj = adapter.py_obj

        if isinstance(key, (types.IntType, types.LongType)):
            py_obj[key] = value
        elif isinstance(key, types.UnicodeType):
            if key[:1] != unicode("_") and hasattr(py_obj, key):
                attr = getattr(py_obj, key)
                if not callable(attr):
                    setattr(py_obj, key, value)
        else:
            raise AssertionError("Invalid key: %r" % key)

        return JS_TRUE
    except:
        traceback.print_exc()
        return report_python_error(cx)


cdef void __finalize_callback__(JSContext* cx, JSObject* js_obj):
    cdef Context pycx
    cdef ObjectAdapter py_obj

    try:
        if not js_context_has_data(cx):
            raise JSError("Unknown JSContext object.")
        
        if not js_object_has_data(cx, js_obj):
            raise JSError("Unknown JSObject.")
        
        pycx = js_context_fetch(cx)
        py_obj = js_object_destroy(cx, js_obj)
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

    def __cinit__(ClassAdapter self, Context cx, ObjectAdapter parent, py_class, bind_constructor, is_global, flags):
        cdef JSObject* proto
        
        self.cx = cx
        self.parent = parent
        self.py_class = py_class
        
        if hasattr(py_class, "__jsname__"):
            name = py_class.__jsname__
        else:
            name = js_classname(py_class)

        self.js_class = <JSClass*> xmalloc(sizeof(JSClass))
        self.js_class.name = <char*> xmalloc((len(name) + 1) * sizeof(char))
        strcpy(self.js_class.name, name)

        self.js_class.flags = flags | JSCLASS_HAS_PRIVATE
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
            proto = JS_InitClass(self.cx.cx, parent.js_obj, NULL, self.js_class,
                            __constructor_callback__, 0, NULL, NULL, NULL, NULL)
            if proto == NULL:
                raise JSError("Failed to bind Python adapter class.")
            js_object_attach(cx.cx, proto, <PyObject*> self)

    def __repr__(self):
        return "<spidermonkey.ClassAdapter: %r>" % self.py_class

    def __dealloc__(self):
        if self.js_class:
            if self.js_class.name:
                free(self.js_class.name)
            free(self.js_class)
