
cdef class Function:
    cdef Context cx
    cdef jsval func
    
    def __init__(Function self, Context cx):
        self.cx = cx
    
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
                raise JSError(self.cx.error)
        finally:
            free(argv)

        retval = js2py(self.cx, rval)
        self.cx.gc.run_maybe()
                
        return retval

    def __dealloc__(Function self):
        JS_RemoveRoot(self.cx.cx, &self.func)


cdef class FunctionAdapter:
    cdef Context cx
    cdef JSObject* js_obj
    cdef object py_obj

    def __cinit__(FunctionAdapter self, Context cx, object obj):
        self.cx = cx
        self.py_obj = obj

    def __repr__(self):
        return "<spidermonkey.FunctionAdapter: %r>" % self.py_obj


cdef JSBool __bound_method_callback__(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval):
    cdef Context pycx
    cdef ObjectAdapter oa
    cdef JSFunction* jsfunc
    cdef int i

    try:
        if not js_context_has_data(cx):
            raise JSError("Unknown JSContext object.")
        
        pycx = js_context_fetch(cx)
        
        if not js_object_has_data(cx, obj):
            return JS_FALSE
        
        oa = js_object_fetch(cx, obj)
        
        jsfunc = JS_ValueToFunction(cx, argv[-2])
        method_name = JS_GetFunctionName(jsfunc)
        method = getattr(oa.py_obj, method_name)

        args = [None] * argc
        for i from 0 <= i < argc:
            args[i] = js2py(pycx, argv[i])

        py_rval = method(*args)
        rval[0] = py2js(pycx, py_rval, obj)
        
        return JS_TRUE
    except:
        return report_python_error(cx)

cdef JSBool __function_callback__(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval):
    cdef Context pycx
    cdef FunctionAdapter fa
    cdef JSFunction* jsfunc
    cdef JSObject* jsobj
    cdef int i

    try:        
        if not js_context_has_data(cx):
            raise JSError("Unknown JSContext object.")

        pycx = js_context_fetch(cx)
        jsfunc = JS_ValueToFunction(cx, argv[-2])
        jsobj = JS_GetFunctionObject(jsfunc)

        if not js_function_has_data(cx, jsobj):
            raise JSError("Function call back without attached functor.")
        
        fa = js_function_fetch(cx, jsobj)
        
        args = [None] * argc
        for i from 0 <= i < argc:
            args[i] = js2py(pycx, argv[i])

        py_rval = fa.py_obj(*args)
        rval[0] = py2js(pycx, py_rval, obj)

        return JS_TRUE
    except:
        return report_python_error(cx)

def js_is_function(Context cx, jsval jsv):
    return JS_TypeOfValue(cx.cx, jsv) == JSTYPE_FUNCTION

def py_is_function(Context cx, object py_obj):
    return isinstance(py_obj, (types.FunctionType, types.LambdaType))

def py_is_bound_method(Context cx, object py_obj):
    return isinstance(py_obj, types.MethodType)

cdef object js2py_function(Context cx, jsval jsv):
    cdef Function ret
    ret = Function(cx)
    ret.func = jsv
    JS_AddRoot(cx.cx, &ret.func)
    return ret

cdef jsval py2js_bound_method(Context cx, object py_obj, JSObject* parent):
    cdef FunctionAdapter fa
    cdef JSFunction* func
    cdef JSObject* obj
    
    if hasattr(py_obj, "func_name"):
        name = py_obj.func_name
    elif hasattr(py_obj, "im_fun") and hasattr(py_obj.im_fun, "func_name"):
        name = py_obj.im_fun.func_name
    else:
        raise JSError("Failed to find function name.")

    func = JS_NewFunction(cx.cx, __bound_method_callback__, 0, 0, NULL, name)
    obj = JS_GetFunctionObject(func)
    return OBJECT_TO_JSVAL(obj)

cdef jsval py2js_function(Context cx, object py_obj, JSObject* parent):
    cdef FunctionAdapter fa
    cdef JSFunction* func
    cdef JSObject* obj
    cdef jsval slot
    
    if hasattr(py_obj, "func_name"):
        name = py_obj.func_name
    elif hasattr(py_obj, "im_fun") and hasattr(py_obj.im_fun, "func_name"):
        name = py_obj.im_fun.func_name
    else:
        raise JSError("Failed to find function name.")
    
    func = JS_NewFunction(cx.cx, __function_callback__, 0, 0, NULL, name)
    fa = FunctionAdapter(cx, py_obj)
    fa.js_obj = JS_GetFunctionObject(func)
    
    js_function_attach(cx.cx, fa.js_obj, <PyObject*> fa)

    return OBJECT_TO_JSVAL(fa.js_obj)
    