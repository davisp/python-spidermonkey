# This Python module is written in the Pyrex language.

# ************************************************************************
# This is an alpha release.  It has many known faults, and interfaces will
# change.

# Note that code listed with JavaScript error messages can be the WRONG
# CODE!  Don't take it seriously.
# ************************************************************************

# spidermonkey 0.0.1a: Python / JavaScript bridge.
# Copyright (C) 2003  John J. Lee <jjl@pobox.com>

# Partly derived from Spidermonkey.xs, Copyright (C) 2001, Claes
#  Jacobsson, All Rights Reserved (Perl Artistic License).
# Partly derived from PerlConnect (part of spidermonkey) Copyright (C)
#  1998 Netscape Communications Corporation (GPL).
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


# Security constraints:
#  Can't add attributes to Python objects.
#  *Can* add JS properties to JS proxies of Python objects, as long as it
#   won't hide a Python attribute of the same name.
#  Can't assign to Python methods.
#  Can't access any Python attribute starting with "_".
#  Also, bound Python objects can of course implement their own
#   protection, as the Window class does.

# XXX
# Does JS expect some exceptions on reading a non-existent property?
#  Currently, this code has the effect of returning undef in that case,
#  which seems odd.  Look in O'Reilly book / spidermonkey code.
# Exception propagation.  IIRC, I'm waiting for except * to be fixed in
#  Pyrex.
# Code listings on JS errors are wrong.
# Review code
#  Replace Object with Py_INCREF and just keeping python object in JS
#   object (need to think first, though, to check this will work).
#  API call return values and bad function arguments!
#  Memory management.
#   Check strcpy's, malloc's / free's / Destroy*'s.
#   Look for and fix any memory leaks.
#  To prevent people crashing module, would be nice to make some things
#   private.  Ask on Pyrex list.
#  Make use of JS API security model?
#  GC issues (see spidermonkey GC tips page).
#  Threading issues (see GC tips again).


import sys
import traceback
import inspect
from cStringIO import StringIO
from types import FloatType, IntType, LongType, DictType, LambdaType, \
     MethodType, FunctionType, StringType, StringTypes, ListType, TupleType

# TODO: A number of these typedefs are wrong, which could cause
# problems in the future.

ctypedef int jsuint
ctypedef int jsint
ctypedef int uint8
ctypedef int uint16
ctypedef int uint32
ctypedef int uintN
ctypedef int size_t
ctypedef int JSBool
IF UNAME_MACHINE == "x86_64":
    ctypedef long jsval
ELSE:
    ctypedef int jsval
ctypedef int jsid
ctypedef int jsword

ctypedef char jschar  # XXX correct?  shouldn't this be wchar_t or something?

ctypedef double jsdouble

cdef extern from "string.h":
    cdef char *strcpy(char *restrict, char *restrict)


cdef extern from "stdlib.h":
    cdef void *malloc(size_t size)
    cdef void free(void *mem)

cdef extern from "Python.h": 
    void Py_INCREF(object)
    void Py_DECREF(object)
    void Py_XINCREF(object)
    void Py_XDECREF(object)

cdef extern from "jsapi.h":
    cdef struct JSRuntime
    cdef struct JSContext
    cdef struct JSObject
    cdef struct JSFunction
    cdef struct JSFunctionSpec
    cdef struct JSString
    cdef struct JSObjectOps
    cdef struct JSXDRState
    cdef struct JSPropertySpec

    cdef enum JSType:
        JSTYPE_VOID  # undefined
        JSTYPE_OBJECT  # object
        JSTYPE_FUNCTION  # function
        JSTYPE_STRING  # string
        JSTYPE_NUMBER  # number
        JSTYPE_BOOLEAN  # boolean
        JSTYPE_LIMIT

    cdef JSType JS_TypeOfValue(JSContext* cx, jsval v)

    cdef enum JSAccessMode:
        JSACC_PROTO = 0  # XXXbe redundant w.r.t. id
        JSACC_PARENT = 1  # XXXbe redundant w.r.t. id
        JSACC_IMPORT = 2  # import foo.bar
        JSACC_WATCH = 3  # a watchpoint on object foo for id 'bar'
        JSACC_READ = 4  # a "get" of foo.bar
        JSACC_WRITE = 8  # a "set" of foo.bar = baz
        JSACC_LIMIT

    cdef struct JSClass

    ctypedef JSBool (* JSPropertyOp)(JSContext *cx, JSObject *obj,
                                     jsval id, jsval *vp)
    ctypedef JSBool (* JSEnumerateOp)(JSContext *cx, JSObject *obj)
    ctypedef JSBool (* JSResolveOp)(JSContext *cx, JSObject *obj, jsval id)
    ctypedef JSBool (* JSConvertOp)(JSContext *cx, JSObject *obj, JSType type,
                                    jsval *vp)
    ctypedef void (* JSFinalizeOp)(JSContext *cx, JSObject *obj)

    ctypedef JSObjectOps *(* JSGetObjectOps)(JSContext *cx, JSClass *clasp)
    ctypedef JSBool (* JSCheckAccessOp)(JSContext *cx, JSObject *obj, jsval id,
                                        JSAccessMode mode, jsval *vp)
    ctypedef JSBool (* JSXDRObjectOp)(JSXDRState *xdr, JSObject **objp)
    ctypedef JSBool (* JSHasInstanceOp)(JSContext *cx, JSObject *obj, jsval v,
                                       JSBool *bp)
    ctypedef uint32 (* JSMarkOp)(JSContext *cx, JSObject *obj, void *arg)

    ctypedef JSBool (*JSNative)(JSContext *cx, JSObject *obj, uintN argc,
                                jsval *argv, jsval *rval)

    cdef struct JSClass:
        char *name
        uint32 flags

        # Mandatory non-null function pointer members.
        JSPropertyOp addProperty
        JSPropertyOp delProperty
        JSPropertyOp getProperty
        JSPropertyOp setProperty
        JSEnumerateOp enumerate
        JSResolveOp resolve
        JSConvertOp convert
        JSFinalizeOp finalize

        # Optionally non-null members start here.
        JSGetObjectOps getObjectOps
        JSCheckAccessOp checkAccess
        JSNative call
        JSNative construct
        JSXDRObjectOp xdrObject
        JSHasInstanceOp hasInstance
        JSMarkOp mark
        # This is actually a function pointer; see MDC.  However, it's
        # optional and we're not using it, so I'm just setting it to a
        # void pointer. -AV
        void *reserveSlots

    cdef JSBool JS_PropertyStub(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
    cdef JSBool JS_EnumerateStub(JSContext *cx, JSObject *obj)
    cdef JSBool JS_ResolveStub(JSContext *cx, JSObject *obj, jsval id)
    cdef JSBool JS_ConvertStub(JSContext *cx, JSObject *obj, JSType type, jsval *vp)
    cdef void JS_FinalizeStub(JSContext *cx, JSObject *obj)

    cdef struct JSFunctionSpec:
        char *name
        JSNative call
        uint8 nargs
        uint8 flags
        uint16 extra  # number of arg slots for local GC roots

    cdef struct JSIdArray:
        jsint length
        jsid  vector[1]  # actually, length jsid words

    cdef JSRuntime *JS_NewRuntime(uint32 maxbytes)
    cdef void JS_DestroyRuntime(JSRuntime *rt)
    cdef JSContext *JS_NewContext(JSRuntime *rt, size_t stackChunkSize)
    cdef void JS_DestroyContext(JSContext *cx)
    cdef JSBool JS_InitStandardClasses(JSContext *cx, JSObject *obj)
    cdef JSObject *JS_InitClass(JSContext *cx, JSObject *obj,
                                JSObject *parent_proto, JSClass *clasp,
                                JSNative constructor, uintN nargs,
                                JSPropertySpec *ps, JSFunctionSpec *fs,
                                JSPropertySpec *static_ps,
                                JSFunctionSpec *static_fs)
    cdef JSBool JS_DefineFunctions(JSContext *cx, JSObject *obj,
                                   JSFunctionSpec *fs)
    cdef JSFunction *JS_DefineFunction(JSContext *cx, JSObject *obj,
                                       char *name, JSNative call,
                                       uintN nargs, uintN attrs)
    cdef JSObject *JS_DefineObject(JSContext *cx, JSObject *obj, char *name,
                                   JSClass *clasp, JSObject *proto,
                                   uintN attrs)
    cdef JS_AddNamedRoot(JSContext *cx, void *rp, char *name)

    cdef JSObject *JS_GetGlobalObject(JSContext *cx)
    cdef JSString *JS_NewStringCopyN(JSContext *cx, char *s, size_t n)

    cdef JSBool JS_CallFunctionName(
        JSContext *cx, JSObject *obj, char *name, uintN argc,
        jsval *argv, jsval *rval) except *
    cdef JSBool JS_CallFunctionValue(
        JSContext *cx, JSObject *obj, jsval fun, uintN argc,
        jsval *argv, jsval *rval) except *

    cdef void JS_free(JSContext *cx, void *p)
    cdef JSBool JS_EvaluateScript(JSContext *cx, JSObject *obj,
                                  char *bytes, uintN length,
                                  char *filename, uintN lineno,
                                  jsval *rval)
    cdef jsdouble *JS_NewDouble(JSContext *cx, jsdouble d)
    cdef JSBool JS_NewDoubleValue(JSContext *cx, jsdouble d, jsval *rval)
    cdef JSObject *JS_NewObject(JSContext *cx, JSClass *clasp,
                                JSObject *proto, JSObject *parent)
    cdef JSObject *JS_NewArrayObject(JSContext *cx, jsint length,
                                     jsval *vector)
    JSFunction *JS_NewFunction(JSContext *cx, JSNative call, uintN nargs,
                               uintN flags, JSObject *parent, char *name)
    JSObject *JS_GetFunctionObject(JSFunction *fun)

    void *JS_GetPrivate(JSContext *cx, JSObject *obj)

    JSBool JS_SetPrivate(JSContext *cx, JSObject *obj, void *data)


    cdef void JS_GC(JSContext *cx)
    cdef void JS_MaybeGC(JSContext *cx)

    cdef enum:
        JS_TRUE
        JS_FALSE
        JSVAL_VOID
        JSVAL_INT
        JSPROP_ENUMERATE
        JSPROP_READONLY
        JS_CLASS_NO_INSTANCE
        JSCLASS_HAS_PRIVATE

    cdef JSClass *JS_GetClass(JSContext *cx, JSObject *obj)

    cdef JSBool JSVAL_IS_OBJECT(jsval v)
    cdef JSBool JSVAL_IS_NUMBER(jsval v)
    cdef JSBool JSVAL_IS_INT(jsval v)
    cdef JSBool JSVAL_IS_DOUBLE(jsval v)
    cdef JSBool JSVAL_IS_STRING(jsval v)
    cdef JSBool JSVAL_IS_BOOLEAN(jsval v)
    cdef JSBool JSVAL_IS_NULL(jsval v)
    cdef JSBool JSVAL_IS_VOID(jsval v)
    cdef JSBool JSVAL_IS_PRIMITIVE(jsval v)
    cdef int JSVAL_TAG(jsval v)
    cdef void JSVAL_SETTAG(jsval v, int t)
    cdef void JSVAL_CLRTAG(jsval v)

    cdef JSBool JSVAL_TO_BOOLEAN(jsval v)
    cdef JSBool JSVAL_IS_GCTHING(jsval v)
    cdef JSBool JSVAL_TO_GCTHING(jsval v)
    cdef JSObject *JSVAL_TO_OBJECT(jsval v)
    cdef int JSVAL_TO_INT(jsval v)
    cdef jsdouble *JSVAL_TO_DOUBLE(jsval v)
    cdef JSString *JSVAL_TO_STRING(jsval v)
    cdef jsval OBJECT_TO_JSVAL(JSObject *obj)
    cdef jsval DOUBLE_TO_JSVAL(jsdouble *dp)
    cdef jsval STRING_TO_JSVAL(JSString *str)
    cdef jsval BOOLEAN_TO_JSVAL(JSBool b)
    cdef jsval INT_TO_JSVAL(int i)

    cdef void *JSVAL_TO_PRIVATE(jsval v)
    cdef jsval PRIVATE_TO_JSVAL(void *p)

    cdef JSFunction *JS_ValueToFunction(JSContext *cx, jsval v)
    cdef char *JS_GetFunctionName(JSFunction *fun)

    cdef char *JS_GetStringBytes(JSString *str)
    cdef size_t JS_GetStringLength(JSString *str)

    cdef JSBool JS_GetArrayLength(JSContext *cx, JSObject *obj,
                                  jsuint *lengthp)
    cdef JSBool JS_IsArrayObject(JSContext *cx, JSObject *obj)
    cdef JSBool JS_GetElement(JSContext *cx, JSObject *obj,
                              jsint index, jsval *vp)
    cdef JSBool JS_SetElement(JSContext *cx, JSObject *obj, jsint index, jsval *vp)
    cdef JSBool JS_DefineElement(JSContext *cx, JSObject *obj, jsint index,
                            jsval value, JSPropertyOp getter,
                            JSPropertyOp setter, uintN attrs)
    cdef JSIdArray *JS_Enumerate(JSContext *cx, JSObject *obj)
    cdef JSBool JS_IdToValue(JSContext *cx, jsid id, jsval *vp)
    cdef void JS_DestroyIdArray(JSContext *cx, JSIdArray *ida)

    cdef JSBool JS_ObjectIsFunction(JSContext *cx, JSObject *obj)

    cdef JSBool JS_GetProperty(JSContext *cx, JSObject *obj,
                               char *name, jsval *vp)
    cdef JSBool JS_DefineProperty(JSContext *cx, JSObject *obj,
                                  char *name, jsval value,
                                  JSPropertyOp getter, JSPropertyOp setter,
                                  uintN attrs)

    cdef void *JS_GetContextPrivate(JSContext *cx)
    cdef void JS_SetContextPrivate(JSContext *cx, void *data)

    cdef JSBool JS_IsExceptionPending(JSContext *cx)
    cdef JSBool JS_GetPendingException(JSContext *cx, jsval *vp)
    cdef void JS_SetPendingException(JSContext *cx, jsval v)
    cdef void JS_ClearPendingException(JSContext *cx)

    cdef struct JSErrorReport:
        char *filename  # source file name, URL, etc., or null
        uintN lineno  # source line number
        char *linebuf  # offending source line without final \n
        char *tokenptr  # pointer to error token in linebuf
        jschar *uclinebuf  # unicode (original) line buffer
        jschar *uctokenptr  # unicode (original) token pointer
        uintN flags  # error/warning, etc.
        uintN errorNumber  # the error number, e.g. see js.msg
        jschar *ucmessage # the (default) error message
        jschar **messageArgs  # arguments for the error message

    ctypedef void (* JSErrorReporter)(JSContext *cx, char *message,
                                      JSErrorReport *report)
    cdef JSErrorReporter JS_SetErrorReporter(JSContext *cx, JSErrorReporter er)
    cdef void JS_ReportError(JSContext *cx, char *format, ...)

    cdef JSRuntime *JS_GetRuntime(JSContext *cx)
    cdef JSContext *JS_ContextIterator(JSRuntime *rt, JSContext **iterp)


cdef extern from "support.c":
    cdef JSClass global_class
    cdef JSObject *make_global_object(JSContext *cx)
    cdef JSFunctionSpec functions[]
    cdef JSBool Print_jsval(JSContext *cx, jsval obj)


cdef class Runtime
cdef class Context
cdef class ProxyClass
cdef class ProxyObject
cdef class ProxyFunction

class JSError(Exception): pass

def compat_isinstance(obj, tuple_or_obj):
    if type(tuple_or_obj) == TupleType:
        for otherobj in tuple_or_obj:
            if isinstance(obj, otherobj):
                return True
        return False
    return isinstance(obj, tuple_or_obj)

def isinteger(x):
    return bool(compat_isinstance(x, (IntType, LongType)))

def isfloat(x):
    return bool(compat_isinstance(x, FloatType))

def isstringlike(x):
    return bool(compat_isinstance(x, StringTypes))

def issequence(x):
    return bool(compat_isinstance(x, (ListType, TupleType)))

def ismapping(x):
    return bool(compat_isinstance(x, DictType))

cdef JSBool report(JSContext *cx):
    # print current traceback, return JS_FALSE
    f = StringIO()
    traceback.print_exc(None, f)
    msg = f.getvalue()
    # JS_ReportError needs %'s escaping
    #JS_ReportError(cx, msg)
    sys.stderr.write(msg)
    return JS_FALSE

# XXX do this properly -- want message in the exception object?
# see also jsc_SetErrorCallbackImpl in JavaScript.xs
cdef void report_error(JSContext *cx, char *message, JSErrorReport *report):
    cdef Context context
    context = get_context(cx)

    lines = context._evaled_script
    #lines = report.linebuf.split("\n")
    if len(lines) == 0:
        msg = "%s:\n<no code>\n" % message
    else:
        i = report.lineno
        n = 2  # lines of context on either side of where error occurred

        start = max(i-n, 0)
        end = min(i+n+1, len(lines))
        nr_pre = i - start
        nr_post = end - i - 1

        data = tuple(lines[start:end])

        outer_fmt = " %s\n"
        inner_fmt = "+%s\n"
        fmt = outer_fmt*nr_pre + inner_fmt + outer_fmt*nr_post
        line = fmt % data
        msg = '\nJavaScript error at line %d: "%s":\n%s\n' % (report.lineno, message, line)
    context._last_error = msg

cdef void *xmalloc(size_t size) except NULL:
    cdef void *mem
    mem = malloc(size)
    if <int>mem == 0:
        raise MemoryError()
    return mem


cdef Runtime get_runtime(JSRuntime *crt):
    cdef Runtime rt

    for rt in RUNTIMES:
        if rt.rt == crt:
            return rt
    raise ValueError("Runtime not found")

cdef Context get_context(JSContext *ccx):
    cdef JSRuntime *crt

    crt = JS_GetRuntime(ccx)
    rt = get_runtime(crt)
    return runtime_get_context(rt, ccx)


RUNTIMES = []

cdef class Runtime:
    cdef JSRuntime *rt
    cdef object _cxs

    def __cinit__(self, *args, **kwargs):
        self.rt = JS_NewRuntime(1000000)
        if self.rt == NULL:
            raise SystemError("can't create JavaScript runtime")

    def __init__(self):
        self._cxs = []
        RUNTIMES.append(self)

    def new_context(self, globj=None):
        cx = Context(self, globj)
        self._cxs.append(cx)
        cx.finish_initialization()
        return cx

    def __del__(self):
        # break cycles
        self._cxs = None

    def __dealloc__(self):
        JS_DestroyRuntime(self.rt)

cdef Context runtime_get_context(Runtime self, JSContext *ccx):
    cdef Context cx

    for cx in self._cxs:
        if ccx == cx.cx:
            return cx
    raise ValueError("JSContext not found in Runtime '%s'" % rt)


cdef class Context:
    cdef JSContext *cx
    cdef JSObject *globj
    cdef Runtime rt
    cdef object _funcs
    cdef object _classes
    cdef object _objs
    cdef object _script
    cdef object _evaled_script
    cdef object _last_error

    def __cinit__(self, Runtime rt, globj):
        # keep ref to Runtime to stop it getting dealloc'd before Context does
        self.rt = rt

        # Tune this to avoid wasting space for shallow stacks, while saving on
        # malloc overhead/fragmentation for deep or highly-variable stacks.
        STACK_CHUNK_SIZE = 8192

        self.cx = JS_NewContext(rt.rt, STACK_CHUNK_SIZE)
        if self.cx == NULL:
            raise SystemError("can't create JavaScript context")

    def __init__(self, Runtime rt, globj):
        cdef ProxyClass proxy_class

        self._funcs = []
        self._classes = []
        self._objs = []
        self._evaled_script = []
        self._last_error = None

        if globj:
            self.bind_class(globj.__class__, False, True)
            proxy_class = context_get_class(self, js_classname(globj))
            self.globj = JS_NewObject(self.cx, proxy_class.jsc, NULL, NULL)
            if self.globj == NULL:
                raise SystemError("can't create JS global object")
            context_register_object(self, globj, self.globj)
        else:
            self.globj = make_global_object(self.cx)
            if self.globj == NULL:
                raise SystemError("can't create JS global object")

    def finish_initialization(self):
        # yuck
        # This function is needed because JS_InitStandardClasses accesses
        # properties of the JS global object, which calls resolve_global,
        # which uses et_context, which in turn looks in runtime._cxs to
        # find the Context.  But the Context can't get added _cxs until it
        # has been constructed, so this can't be in __init__!
        if not JS_InitStandardClasses(self.cx, self.globj):
            raise SystemError("can't initialise standard JS classes")
        JS_SetErrorReporter(self.cx, report_error)
        if not JS_DefineFunctions(self.cx, self.globj, functions):
            raise SystemError("can't define JavaScript functions")

    def get_global(self, name):
        """Only works with undotted names.  Use eval_script for anything
        else."""
        # XXXX probably best to get rid of this -- eval_script does the job
        cdef jsval jsv
        if not isstringlike(name):
            raise TypeError("name must be string-like")

        if not JS_GetProperty(self.cx, self.globj, name, &jsv):
            raise SystemError("can't get JavaScript property")
        val = Py_from_JS(self.cx, jsv)
        if val is None:
            raise ValueError("no global named '%s'" % name)
##         # XXXX why does this hang??
##             raise AttributeError("%s instance has no attribute '%s'" %
##                                  (self.__class__.__name__, name))
        else:
            return val

    def bind_callable(self, name, function):
        if not callable(function):
            raise ValueError("not a callable object")

        self._funcs.append((name, function))
        JS_DefineFunction(self.cx, self.globj, name, function_cb, 0, 0)

    def bind_attribute(self, name, obj, attr_name):
        cdef jsval initial_value
        if not isstringlike(name):
            raise TypeError("name must be string-like")
        if not isstringlike(attr_name):
            raise TypeError("name must be string-like")

        initial_value = JS_from_Py(self.cx, NULL, getattr(obj, name))
        JS_DefineProperty(self.cx, self.globj, name, initial_value,
                          get_property, set_property, 0)

    def bind_class(self, klass, bind_constructor=True, is_global=False,
                   flags=0):
        if not inspect.isclass(klass): raise TypeError("klass must be a class")
        if not isinteger(flags): raise TypeError("flags must be an integer")

        c = ProxyClass(self, klass, bind_constructor, is_global, flags)
        self._classes.append(c)

    def bind_object(self, name, obj):
        cdef JSObject *jsobj
        cdef ProxyClass proxy_class

        if not isstringlike(name):
            raise TypeError("name must be string-like")

        proxy_class = context_get_class(self, js_classname(obj))
        jsobj = NULL
        jsobj = JS_DefineObject(self.cx, self.globj, name, proxy_class.jsc,
                                NULL, JSPROP_READONLY)
        if jsobj != NULL:
            context_register_object(self, obj, jsobj)
        else:
            raise ValueError("failed to bind Python object %s" % obj)

    def call_fn(self, name, *args):
        cdef jsval *argv
        cdef jsval rval
        cdef jsval jsarg

        if not isstringlike(name):
            raise TypeError("name must be string-like")

        nr_args = len(args)
        argv = <jsval *>xmalloc(sizeof(jsval)*nr_args)
        try:
            for i from 0 <= i < nr_args:
                arg = args[i]
                # XXX shouldn't NULL be the JSObject if it's a Python class
                # instance?
                jsarg = JS_from_Py(self.cx, NULL, arg)
                argv[i] = jsarg

            ok = JS_CallFunctionName(self.cx, self.globj,
                                     name, nr_args, argv, &rval)
        finally:
            free(argv)

        if not ok:
            raise ValueError("no function named '%s'" % name)
        retval = Py_from_JS(self.cx, rval)
        JS_MaybeGC(self.cx)
        return retval

    def eval_script(self, script):
        cdef jsval rval

        if not isstringlike(script):
            raise TypeError("name must be string-like")

        self._script = script
        self._evaled_script.extend(script.split("\n"))

        # here!
        if not JS_EvaluateScript(self.cx, self.globj,
                                 script, len(script), "Python", 0, &rval):
            raise JSError("can't evaluate JavaScript script: %s" % self._last_error)

        retval = Py_from_JS(self.cx, rval)
        JS_MaybeGC(self.cx)
        return retval

    def __dealloc__(self):
        JS_DestroyContext(self.cx)

cdef void context_register_object(Context self, obj, JSObject *jsobj):
    cdef ProxyObject proxy_obj

    proxy_obj = ProxyObject(obj)
    proxy_obj.jsobj = jsobj
    context_add_object(self, proxy_obj)

cdef void context_add_object(Context self, ProxyObject obj):
    self._objs.append(obj)

cdef void context_remove_object(Context self, ProxyObject obj):
    self._objs.remove(obj)

cdef object context_get_object(Context self, JSObject *cobj):
    cdef ProxyObject obj
    for obj in self._objs:
        if cobj == obj.jsobj:
            return obj
    raise ValueError("object not found in context")

cdef object context_get_object_from_obj(Context self, object pyobj):
    cdef ProxyObject obj
    for obj in self._objs:
        if pyobj == obj.obj:
            return obj
    raise ValueError("object not found in context")

cdef context_get_callback_fn(Context self, name):
    for cbname, fn in self._funcs:
        if cbname == name:
            return fn
    raise ValueError("no callback function named '%s' is bound" % name)

cdef ProxyClass context_get_class(Context self, name):
    cdef ProxyClass cl
    for cl in self._classes:
        if cl.jsc.name == name:
            return cl
    raise ValueError("no class named '%s' is bound" % name)


cdef class ProxyClass:
    cdef JSClass *jsc
    cdef object klass

    def __cinit__(self, Context context, klass, bind_constructor, is_global,
                flags):
        cdef JSObject *base_obj

        self.jsc = <JSClass *> xmalloc(sizeof(JSClass))
        name = js_classname(klass)
        self.jsc.name = <char *> xmalloc((len(name)+1)*sizeof(char))
        strcpy(self.jsc.name, name)

        # not using Get/SetPrivate ATM
        #self.jsc.flags = JSCLASS_HAS_PRIVATE
        self.jsc.flags = flags
        self.jsc.addProperty = JS_PropertyStub
        self.jsc.delProperty = JS_PropertyStub
        self.jsc.getProperty = get_property
        self.jsc.setProperty = set_property
        self.jsc.enumerate = JS_EnumerateStub
        if is_global:
            self.jsc.resolve = resolve_global
        else:
            self.jsc.resolve = JS_ResolveStub
        self.jsc.convert = JS_ConvertStub
        self.jsc.finalize = finalize

        self.jsc.getObjectOps = NULL
        self.jsc.checkAccess = NULL
        self.jsc.call = NULL
        self.jsc.construct = NULL
        self.jsc.xdrObject = NULL
        self.jsc.hasInstance = NULL
        self.jsc.mark = NULL
        self.jsc.reserveSlots = NULL

        if bind_constructor:
            if JS_InitClass(
                context.cx, context.globj, NULL, self.jsc, constructor_cb, 0,
                NULL, NULL, NULL, NULL) == NULL:
                raise SystemError("couldn't initialise JavaScript proxy class")

    def __init__(self, Context context, klass, bind_constructor, is_global,
                 flags):
        self.klass = klass

    def __dealloc__(self):
        free(self.jsc.name)
        free(self.jsc)

def trap(): pass

from types import InstanceType

def js_classname(pobj):
    # class or instance?
    if inspect.isclass(pobj):
        klass = pobj
    elif hasattr(pobj, "__class__"):
        klass = pobj.__class__
    else:
        raise AttributeError("%s has no detectable class." % pobj)

    try:
        name = klass.js_name
    except AttributeError:
        name = klass.__name__

    if not isstringlike(name):
        raise AttributeError("%s js_name attribute is not string-like" % klass)
    return name


cdef class ProxyObject:
    cdef JSObject *jsobj
    cdef object obj

    def __init__(self, obj):
        self.obj = obj

cdef class ProxyFunction:
    cdef Context cx
    cdef jsval fun

    def __init__(self, Context cx):
        self.cx = cx

    def __call__(self, *args):
        cdef jsval *argv
        cdef jsval rval
        cdef jsval jsarg

        nr_args = len(args)
        argv = <jsval *>xmalloc(sizeof(jsval)*nr_args)
        try:
            for i from 0 <= i < nr_args:
                arg = args[i]
                # XXX shouldn't NULL be the JSObject if it's a Python class
                # instance?
                jsarg = JS_from_Py(self.cx.cx, NULL, arg)
                argv[i] = jsarg

            JS_CallFunctionValue(self.cx.cx, self.cx.globj, self.fun, nr_args, argv, &rval)
        finally:
            free(argv)

        if not ok:
            raise RuntimeError(self.cx._last_error)
        retval = Py_from_JS(self.cx.cx, rval)
        JS_MaybeGC(self.cx.cx)
        return retval

cdef ProxyFunction create_proxy_function(Context cx, jsval fun):
    if JS_TypeOfValue(cx.cx, fun) != JSTYPE_FUNCTION:
        raise JSError("Value is not a function: %s" % JS_TypeOfValue(cx.cx, fun))
    cdef ProxyFunction ret
    ret = ProxyFunction(cx)
    ret.fun = fun
    return ret

cdef JSBool constructor_cb(JSContext *cx, JSObject *obj,
                           uintN argc, jsval *argv, jsval *rval):
    cdef ProxyClass proxy_class
    cdef Context context
    cdef char *fname
    cdef JSFunction *func
    cdef int arg
    cdef ProxyObject object

    func = JS_ValueToFunction(cx, argv[-2])
    if func == NULL:
        msg = "couldn't get JS constructor"
        JS_ReportError(cx, msg)
        return JS_FALSE
    fname = JS_GetFunctionName(func)

    try:
        context = get_context(cx)
        proxy_class = context_get_class(context, fname)

        args = []
        for arg from 0 <= arg < argc:
            args.append(Py_from_JS(cx, argv[arg]))
        if not hasattr(proxy_class.klass, "js_constructor"):
            py_rval = proxy_class.klass(*args)
        else:
            py_rval = proxy_class.klass.js_constructor[0](context, *args)
        context_register_object(context, py_rval, obj)
    except:
        return report(cx)

    return JS_TRUE


cdef JSBool bound_method_cb(JSContext *cx, JSObject *obj, uintN argc,
                            jsval *argv, jsval *rval):
    cdef Context context
    cdef JSFunction *func
    cdef int arg
    cdef JSClass *jsclass
    cdef ProxyObject proxy_obj

    func = JS_ValueToFunction(cx, argv[-2])
    jsclass = JS_GetClass(cx, obj)

    try:
        context = get_context(cx)
        proxy_obj = context_get_object(context, obj)
        method_name = JS_GetFunctionName(func)

        py_args = []
        for arg from 0 <= arg < argc:
            py_args.append(Py_from_JS(cx, argv[arg]))
        meth = getattr(proxy_obj.obj, method_name)
        py_rval = meth(*py_args)

        rval[0] = JS_from_Py(cx, obj, py_rval)
    except:
        return report(cx)

    return JS_TRUE


cdef jsval new_method(JSContext *cx, object py_method):
    cdef JSFunction *method
    cdef JSObject *method_obj

    # Method (hence its func_name attribute) will go away only when the
    # context is GC'd by Python, because the Python function object is
    # stored in Context extension type instance.  XXX erm, no it isn't:
    # the class instance is kept in the ProxyClass, but the instance's
    # function is not.  Hmm...  XXX also, JS objects exist in runtime,
    # not in context!
    py_method_name = py_method.func_name
    method = JS_NewFunction(cx, bound_method_cb, 0, 0, NULL, py_method_name)
    method_obj = JS_GetFunctionObject(method)
    return OBJECT_TO_JSVAL(method_obj)


cdef JSBool resolve_global(JSContext *cx, JSObject *obj, jsval id):
    cdef Context context
    cdef ProxyObject proxy_obj
    cdef object thing
    cdef object key

    try:
        context = get_context(cx)
        # XXXX need to prohibit anything starting with "_"?  Don't think so...
        proxy_obj = context_get_object(context, obj)

        thing = proxy_obj.obj
        key = Py_from_JS(cx, id)
        if type(key) == StringType and hasattr(thing, key):
            attr = getattr(thing, key)
            if compat_isinstance(attr, MethodType):
                context.bind_callable(key, attr)
            else:
                context.bind_attribute(key, thing, key)
        return JS_TRUE
    except:
        return report(cx)
    return JS_FALSE


cdef void finalize(JSContext *cx, JSObject *obj):
    cdef Context context
    cdef ProxyObject proxy_obj

    try:
        context = get_context(cx)
        proxy_obj = context_get_object(context, obj)
        context_remove_object(context, proxy_obj)
    except:
        report(cx)


cdef JSBool get_property(JSContext *cx, JSObject *obj, jsval id, jsval *vp):
    cdef Context context
    cdef ProxyObject proxy_obj
    cdef object thing
    cdef object key

    try:
        context = get_context(cx)
        proxy_obj = context_get_object(context, obj)

        thing = proxy_obj.obj
        key = Py_from_JS(cx, id)

        if type(key) == IntType:
            try:
                attr = thing[key]
            except:
                pass
            else:
                vp[0] = JS_from_Py(cx, obj, attr)
        elif type(key) == StringType:
            if not key.startswith("_"):
                try:
                    attr = getattr(thing, key)
                except:
                    pass
                else:
                    vp[0] = JS_from_Py(cx, obj, attr)
        elif key is None:
            # It's the magic XML namespace id:
            # http://mxr.mozilla.org/mozilla/source/js/src/jsapi.h#1382
            vp[0] = JSVAL_VOID
        else:
            raise AssertionError("Illegal key: %s" % key)
        return JS_TRUE
    except:
        return report(cx)
    return JS_FALSE

cdef JSBool set_property(JSContext *cx, JSObject *obj, jsval id, jsval *vp):
    cdef Context context
    cdef ProxyObject proxy_obj
    cdef object thing
    cdef object key
    cdef object value
    cdef object attr

    try:
        context = get_context(cx)
        proxy_obj = context_get_object(context, obj)

        thing = proxy_obj.obj
        key = Py_from_JS(cx, id)
        value = Py_from_JS(cx, vp[0])
        if type(key) == IntType:
            try:
                thing[key] = value
            except:
                pass
        elif type(key) == StringType:
            if hasattr(thing, key) and not key.startswith("_"):
                attr = getattr(thing, key)
                if not callable(attr):
                    try:
                        setattr(thing, key, value)
                    except:
                        pass
        else:
            raise AssertionError("Didn't expect key: %s" % key)
        return JS_TRUE
    except:
        return report(cx)

    return JS_FALSE


cdef JSBool function_cb(JSContext *cx, JSObject *obj,
                        uintN argc, jsval *argv, jsval *rval):
    cdef JSFunction *jsfunc
    cdef int i
    cdef char *name

    jsfunc = JS_ValueToFunction(cx, argv[-2])
    if jsfunc == NULL:
        return JS_FALSE
    name = JS_GetFunctionName(jsfunc)
    if name == NULL:
        return JS_FALSE

    try:
        context = get_context(cx)
        callback = context_get_callback_fn(context, name)

        args = [None]*argc
        for i from 0 <= i < argc:
            args[i] = Py_from_JS(cx, argv[i])
        pyrval = callback(*args)

        # XXX shouldn't NULL be the JSObject if it's a Python class instance?
        rval[0] = JS_from_Py(cx, NULL, pyrval)
    except:
        return report(cx)

    return JS_TRUE


cdef jsval JS_from_Py(JSContext *cx, JSObject *parent, py_obj) except 0:
    # Convert Python value to equivalent JavaScript value.
    cdef JSObject *jsobj
    cdef JSString *s

    cdef jsval elem
    cdef jsval rval

    cdef int i
    cdef int nr_elems
    cdef jsval *elems
    cdef JSObject *arr_obj

    cdef Context context
    cdef ProxyClass proxy_class
    cdef ProxyObject proxy_obj

    if py_obj is None:
        return JSVAL_VOID
    elif isinteger(py_obj):
        return INT_TO_JSVAL(py_obj)
    elif isfloat(py_obj):
        if not JS_NewDoubleValue(cx, py_obj, &rval):
            raise SystemError("can't create new double")
        return rval
    elif isstringlike(py_obj):
        s = JS_NewStringCopyN(cx, py_obj, len(py_obj))
        if s == NULL:
            raise SystemError("can't create new string")
        return STRING_TO_JSVAL(s)
    elif ismapping(py_obj):
        jsobj = JS_NewObject(cx, NULL, NULL, NULL)

        if jsobj == NULL:
            raise SystemError("can't create new object")
        else:
            # assign properties
            for key, value in py_obj.iteritems():
                elem = JS_from_Py(cx, parent, value)
                ok = JS_DefineProperty(cx, jsobj, key, elem, NULL, NULL,
                                       JSPROP_ENUMERATE)
                if not ok:
                    raise SystemError("can't define property")

            return OBJECT_TO_JSVAL(jsobj)

    elif issequence(py_obj):
        arr_obj = JS_NewArrayObject(cx, 0, NULL)

        if arr_obj == NULL:
            raise SystemError("can't create new array object")
        else:
            for i from 0 <= i < len(py_obj):
                elem = JS_from_Py(cx, parent, py_obj[i])
                ok = JS_DefineElement(cx, arr_obj, i, elem, NULL, NULL,
                                      JSPROP_ENUMERATE)
                if not ok:
                    raise SystemError("can't define element")

        return OBJECT_TO_JSVAL(arr_obj)
    elif compat_isinstance(py_obj, MethodType):
        # XXX leak? -- calls JS_DefineFunction every time a method
        #  is called
        return new_method(cx, py_obj)
    elif compat_isinstance(py_obj, (FunctionType, LambdaType)):
        # XXX implement me?
        return JSVAL_VOID
    else:
        # If we get here, py_obj is probably a Python class or a class instance.
        # XXXX could problems be caused if some weird object such a module or
        #  regexp or code object were, eg., returned by a Python function?
        context = get_context(cx)

        # is object already bound to a JS proxy?...
        try:
            proxy_obj = context_get_object_from_obj(context, py_obj)
        except ValueError:
            # ...no, so create and register a new JS proxy
            proxy_class = context_get_class(context, js_classname(py_obj))
            # XXXX I have *no idea* if using globj as the proto here is
            #  correct!  If I don't put globj in here, JS does a get_property
            #  on an object that hasn't been registered with the Context --
            #  dunno why.
            jsobj = JS_NewObject(cx, proxy_class.jsc, context.globj, parent)
            if jsobj == NULL:
                raise SystemError("couldn't look up or create new object")
            context_register_object(context, py_obj, jsobj)
        else:
            # ...yes, use existing proxy
            jsobj = proxy_obj.jsobj

        return OBJECT_TO_JSVAL(jsobj)


cdef dict_from_JShash(JSContext *cx, JSObject *hash):
    cdef JSIdArray *prop_arr
    cdef int i
    cdef jsval jskey
    cdef jsval jsvalue
    cdef JSObject *obj

    prop_arr = JS_Enumerate(cx, hash)

    d = {}

    for i from 0 <= i < prop_arr.length:
        JS_IdToValue(cx, (prop_arr.vector)[i], &jskey)

        if JSVAL_IS_STRING(jskey):
            key = JS_GetStringBytes(JSVAL_TO_STRING(jskey))
            JS_GetProperty(cx, hash, key, &jsvalue)
        elif JSVAL_IS_INT(jskey):
            key = JSVAL_TO_INT(jskey)
            JS_GetElement(cx, hash, key, &jsvalue)
        else:
            raise AssertionError("can't happen")

        if JSVAL_IS_PRIMITIVE(jsvalue):
            d[key] = Py_from_JSprimitive(jsvalue)
        else:
            if JSVAL_IS_OBJECT(jsvalue):
                obj = JSVAL_TO_OBJECT(jsvalue)
                if JS_IsArrayObject(cx, obj):
                    d[key] = list_from_JSarray(cx, obj)
                else:
                    d[key] = dict_from_JShash(cx, obj)

    JS_DestroyIdArray(cx, prop_arr)

    return d

cdef list_from_JSarray(JSContext *cx, JSObject *array):
    cdef int nr_elems, i
    cdef jsval elem
    cdef JSObject *jsobj

    JS_GetArrayLength(cx, array, &nr_elems)

    l = [None]*nr_elems

    for i from 0 <= i < nr_elems:
        JS_GetElement(cx, array, i, &elem)

        if JSVAL_IS_PRIMITIVE(elem):
            l[i] = Py_from_JSprimitive(elem)
        elif JSVAL_IS_OBJECT(elem):
            jsobj = JSVAL_TO_OBJECT(elem)
            if JS_IsArrayObject(cx, jsobj):
                l[i] = list_from_JSarray(cx, jsobj)
            else:
                l[i] = dict_from_JShash(cx, jsobj)

    return l

cdef Py_from_JSprimitive(jsval v):
    # JS_NULL is null, JS_VOID is undefined
    if JSVAL_IS_NULL(v) or JSVAL_IS_VOID(v):
        return None
    elif JSVAL_IS_INT(v):
        return JSVAL_TO_INT(v)
    elif JSVAL_IS_DOUBLE(v):
        return (JSVAL_TO_DOUBLE(v)[0])
    elif JSVAL_IS_STRING(v):
        return JS_GetStringBytes(JSVAL_TO_STRING(v))
    elif JSVAL_IS_BOOLEAN(v):
        if JSVAL_TO_BOOLEAN(v):
            return True
        else:
            return False
    else:
        raise SystemError("unknown primitive type")

cdef object Py_from_JS(JSContext *cx, jsval v):
    # Convert JavaScript value to equivalent Python value.
    cdef JSObject *object
    cdef ProxyObject proxy_obj
    cdef Context context

    if JSVAL_IS_PRIMITIVE(v):
        return Py_from_JSprimitive(v)
    elif JSVAL_IS_OBJECT(v):
        object = JSVAL_TO_OBJECT(v)
        context = get_context(cx)

        if JS_IsArrayObject(cx, object):
            return list_from_JSarray(cx, object)
        elif JS_ObjectIsFunction(cx, object):
            return create_proxy_function(context, v);
        else:
            try:
                proxy_obj = context_get_object(context, object)
            except ValueError:
                return dict_from_JShash(cx, object)
            else:
                return proxy_obj.obj
