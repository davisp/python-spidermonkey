
cdef class Function:
    cdef Context cx
    cdef jsval func
    
    def __init__(Function self, Context cx):
        self.cx = cx
    
    def __hash__(self):
        cdef int ret
        ret = self.func
        return ret
    
    def __call__(Function self, *args):
        cdef jsval* argv
        cdef jsval rval
        cdef jsval jsarg

        nr_args = len(args)
        argv = <jsval*> xmalloc(sizeof(jsval) * nr_args)
        try:
            for i from 0 <= i < nr_args:
                arg = args[i]
                jsarg = py2js(self.cx, arg, NULL)
                argv[i] = jsarg

            if not JS_CallFunctionValue(self.cx.cx, self.cx.root.js_obj, self.func, nr_args, argv, &rval):
                raise JSError("Failed to execute function: %s" % self.cx._error)
        finally:
            free(argv)

        retval = js2py(self.cx, rval)
        self.cx.gc.maybe()
        return retval

    def __dealloc__(Function self):
        JS_RemoveRoot(self.cx.cx, &self.func)

cdef JSBool __function_callback__(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval):
    cdef Context pycx
    cdef jsval jsfunc
    cdef int i

    try:
        pycx = js_context_fetch(cx)
        callback = pycx.get_function(js_create_value(argv[-2]))

        args = [None] * argc
        for i from 0 <= i < argc:
            args[i] = js2py(pycx, argv[i])

        py_rval = callback(*args)
        rval[0] = py2js(pycx, py_rval, obj)
    except:
        return report_python_error(cx)

    return JS_TRUE

def js_is_function(Context cx, jsval jsv):
    return JS_TypeOfValue(cx.cx, jsv) == JSTYPE_FUNCTION

def py_is_function(Context cx, object py_obj):
    return callable(py_obj)

cdef object js2py_function(Context cx, jsval jsv):
    cdef Function ret
    ret = Function(cx)
    ret.func = jsv
    cx.gc.add_root(&jsv)
    return ret

cdef jsval py2js_function(Context cx, object py_obj, JSObject* parent):
    cdef JSFunction* func
    cdef JSObject* obj
    cdef jsval rval
    
    func = JS_NewFunction(cx.cx, __function_callback__, 0, 0, parent, NULL)
    obj = JS_GetFunctionObject(func)
    js_object_attach(obj, <PyObject*> py_obj)
    return OBJECT_TO_JSVAL(obj)
