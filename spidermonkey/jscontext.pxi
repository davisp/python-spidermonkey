
cdef class Context:
    cdef JSContext* cx
    cdef Runtime rt
    cdef ObjectAdapter root
    cdef GC gc
    cdef object reg
    cdef object err

    def __cinit__(self, Runtime rt, root):
        self.rt = rt

        STACK_CHUNK_SIZE = 8192

        self.cx = JS_NewContext(rt.rt, STACK_CHUNK_SIZE)
        if self.cx == NULL:
            raise JSError("Failed to create Context")

    def __init__(Context self, Runtime rt, root):
        cdef JSObject* obj

        self.gc = GC(self)
        self.reg = Registry()
        self.err = None

        self.reg = {}

        js_context_attach(self.cx, <PyObject*> self)

        if root:
            ca = self.install_class(root.__class__, False, True)
            self.root = ObjectAdapter(self, ca, None)
            if not self.root:
                raise JSError("Failed to bind global object.")
        else:
            obj = js_make_global_object(self.cx)
            self.root = ObjectAdapter(self, None, None, None)
            js_object_attach(obj, <PyObject*> None)
            self.root.js_obj = obj
            if not self.root:
                raise JSError("Failed to create default global object.")
        
        if not JS_InitStandardClasses(self.cx, self.root.js_obj):
            raise JSError("Failed to initialize standard classes.")
        
        JS_SetErrorReporter(self.cx, __report_error_callback__)
        
    def __dealloc__(self):
        JS_DestroyContext(self.cx)

    def install_class(self, py_class, bind_constructor=True, is_global=False, flags=0):
        """\
        Install a Python class into the JavaScript runtime.
        """
        if not inspect.isclass(klass):
            raise TypeError("Unable to install %r as a class." % klass)
        if not isinteger(flags):
            raise TypeError("Flags is not an integer.")

        c = ClassAdaptor(self, python_class, bind_constructor, is_global, flags)
        self.classes[c.to_value()] = c
        return c

    def bind(Context self, name, obj, parent=None):
        """\
        Attach a Python object to the JavaScript runtime.
        
        You should be able to attach most types of Python objects
        including builtin types, object instances, functions etc. About
        the only thing you can't bind is a Python class which should
        be isntalled into the JavaScript environment using
        Context.install_class
        
        This call will bind the provided object `obj` to the JS root. The
        Python value is also referenced to keep it from being garbage
        collected.
        """
        cdef ClassAdapter ca
        cdef jsval jsv

        if not isinstance(name, types.StringTypes):
            raise TypeError("Name must be a string.")

        ca = self.install_class(obj)
        jsv = py2js(self, obj, self.root.js_obj)
        if not JS_DefineProperty(self.cx, self.root.js_obj, name, jsv,
                                    __get_property_callback__, __set_property_callback__, 0):
            raise JSError("Failed to bind Python object to the global object.")

    def execute(Context self, object script):
        """\
        Execute JavaScript source code.
        """
        cdef jsval rval
        try:
            if not isinstance(script, types.StringTypes):
                raise TypeError("Script must be a string.")

            if not JS_EvaluateScript(self.cx, self.root.js_obj, script, len(script), "Python", 0, &rval):
                raise JSError("Failed to execute script: %s" % self._error)
        
            return js2py(self, rval)
        finally:
            self._gc.maybe()

    def register(Context self, object obj):
        """\
        Register a Python object in the context so that it doesn't
        get garbage collected.
        """
        val = id(obj)
        assert val not in self.reg, "Objected previously registered."
        self.reg[val] = obj
    
    def unregister(Context self, object obj):
        val = id(obj)
        assert val in self.reg, "Objected not registered."
        del self.reg[val]
    
    