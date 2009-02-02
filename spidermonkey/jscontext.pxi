
class JSRootObject(object):
    pass

cdef class Context:
    cdef JSContext* cx
    cdef Runtime rt
    cdef ObjectAdapter root
    cdef GC gc
    cdef object reg
    cdef object classes
    cdef object error

    def __cinit__(self, Runtime rt, root):
        self.rt = rt

        STACK_CHUNK_SIZE = 8192

        self.cx = JS_NewContext(rt.rt, STACK_CHUNK_SIZE)
        if self.cx == NULL:
            raise JSError("Failed to create Context")

    def __init__(Context self, Runtime rt, root):
        cdef ClassAdapter ca
        cdef JSObject* js_obj
        cdef PyObject* py_obj

        self.gc = GC(self)
        self.reg = {}
        self.classes = {}
        self.error = None

        js_context_attach(self.cx, <PyObject*> self)

        if not root:
            root = JSRootObject()
    
        ca = self.install_class(root.__class__, False, True)
        js_obj = JS_NewObject(self.cx, ca.js_class, NULL, NULL)
        if js_obj == NULL:
            raise JSError("Failed to create root object.")
        self.root = ObjectAdapter(self, ca, None, root)
        self.root.js_obj = js_obj
        js_object_attach(self.cx, self.root.js_obj, <PyObject*> self.root)
        
        if not JS_InitStandardClasses(self.cx, js_obj):
            raise JSError("Failed to initialize standard classes.")
        
        JS_SetErrorReporter(self.cx, __report_error_callback__)
        
    def __dealloc__(self):
        JS_DestroyContext(self.cx)

    def install_class(self, py_class, bind_constructor=True, is_global=False, flags=0):
        """\
        Install a Python class into the JavaScript runtime.
        """
        if not inspect.isclass(py_class):
            raise TypeError("Unable to install %r as a class." % py_class)
        if not isinstance(flags, (types.IntType, types.LongType)):
            raise TypeError("Flags is not an integer.")

        if py_class in self.classes:
            return self.classes[py_class]
        
        ca = ClassAdapter(self, self.root, py_class, bind_constructor, is_global, flags)
        self.classes[py_class] = ca
        return ca

    def bind(Context self, name, obj):
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
        cdef PyJSString js_str

        js_str = py2js_jsstring(self.cx, name)

        ca = self.install_class(obj.__class__)
        jsv = py2js(self, obj, self.root.js_obj)
        if not JS_DefineUCProperty(self.cx, self.root.js_obj, js_str.chars(), js_str.length(), jsv,
                                    __get_property_callback__, __set_property_callback__, 0):
            raise JSError("Failed to bind Python object to the global object.")

    def unbind(Context self, name):
        cdef jsval rval
        cdef PyJSString js_str

        js_str = py2js_jsstring(self.cx, name)
        
        ret = self.execute(name + unicode(";")) # yeah yeah, I know.
        if not JS_DeleteUCProperty2(self.cx, self.root.js_obj, js_str.chars(), js_str.length(), &rval):
            raise JSError("Failed to unbind property: %s" % name)
        # This always returns True for some reason
        #return js2py(self, rval)
        return ret

    def execute(Context self, script):
        """\
        Execute JavaScript source code.
        """
        cdef jsval rval
        cdef PyJSString js_str

        js_str = py2js_jsstring(self.cx, script)
        
        try:
            if not JS_EvaluateUCScript(self.cx, self.root.js_obj, js_str.chars(), js_str.length(), "Python", 0, &rval):
                raise JSError(self.error)
            return js2py(self, rval)
        finally:
            self.gc.run_maybe()

    def set_error(self, mesg):
        self.error = mesg
    
    
