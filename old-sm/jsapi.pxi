

ctypedef int uint8
ctypedef int uint16
ctypedef int uint32
ctypedef int uintN
ctypedef int size_t

ctypedef char jschar
ctypedef double jsdouble
ctypedef int jsid
ctypedef int jsint
ctypedef int jsuint
IF UNAME_MACHINE == "x86_64" or UNAME_MACHINE == "amd64":
    ctypedef long jsval
ELSE:
    ctypedef int jsval
ctypedef int JSBool

cdef extern from "stdio.h":
    cdef int printf(char* format, ...)

cdef extern from "string.h":
    cdef char* strcpy(char* restrict, char* restrict)
    cdef void* memcpy(void* dest, void* src, size_t num)

cdef extern from "stdlib.h":
    cdef void* malloc(size_t size)
    cdef void free(void* mem)

cdef extern from "Python.h":
    cdef struct PyObject
    cdef void Py_DECREF(PyObject* o)
    cdef PyObject* PyString_FromStringAndSize(char* buf, size_t length)
    cdef int PyString_AsStringAndSize(PyObject* str, char** buf, int* length)

cdef extern from "jsapi.h":
    cdef struct JSClass
    cdef struct JSContext
    cdef struct JSErrorReport
    cdef struct JSFunction
    cdef struct JSFunctionSpec
    cdef struct JSString
    cdef struct JSObject
    cdef struct JSObjectOps
    cdef struct JSRuntime
    cdef struct JSPropertySpec
    cdef struct JSXDRState

    cdef enum:
        JS_TRUE
        JS_FALSE
        JSVAL_VOID
        JSVAL_INT
        JSPROP_ENUMERATE
        JSPROP_READONLY
        JS_CLASS_NO_INSTANCE
        JSCLASS_HAS_PRIVATE

    cdef enum JSType:
        JSTYPE_VOID
        JSTYPE_OBJECT
        JSTYPE_FUNCTION
        JSTYPE_STRING
        JSTYPE_NUMBER
        JSTYPE_BOOLEAN
        JSTYPE_LIMIT

    cdef enum JSAccessMode:
        JSACC_PROTO = 0
        JSACC_PARENT = 1
        JSACC_IMPORT = 2
        JSACC_WATCH = 3
        JSACC_READ = 4
        JSACC_WRITE = 8
        JSACC_LIMIT

    ctypedef JSBool (*JSPropertyOp) (JSContext* cx, JSObject* obj, jsval id, jsval* vp)
    ctypedef JSBool (*JSEnumerateOp) (JSContext* cx, JSObject* obj)
    ctypedef JSBool (*JSResolveOp) (JSContext* cx, JSObject* obj, jsval id)
    ctypedef JSBool (*JSConvertOp) (JSContext* cx, JSObject* obj, JSType type, jsval* vp)
    ctypedef void (*JSFinalizeOp) (JSContext* cx, JSObject* obj)
    ctypedef JSObjectOps* (*JSGetObjectOps) (JSContext* cx, JSClass* clasp)
    ctypedef JSBool (*JSCheckAccessOp) (JSContext* cx, JSObject* obj, jsval id, JSAccessMode mode, jsval* vp)
    ctypedef JSBool (*JSXDRObjectOp) (JSXDRState* xdr, JSObject** objp)
    ctypedef JSBool (*JSHasInstanceOp) (JSContext* cx, JSObject* obj, jsval v, JSBool* bp)
    ctypedef uint32 (*JSMarkOp) (JSContext* cx, JSObject* obj, void* arg)
    ctypedef JSBool (*JSNative) (JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
    ctypedef void (*JSErrorReporter) (JSContext* cx, char* message, JSErrorReport* report)
    
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
        void* reserveSlots

    cdef struct JSErrorReport:
        char* filename
        uintN lineno
        char* linebuf
        char* tokenptr
        jschar* uclinebuf
        jschar* uctokenptr
        uintN flags
        uintN errorNumber
        jschar* ucmessage
        jschar** messageArgs

    cdef struct JSFunctionSpec:
        char *name
        JSNative call
        uint8 nargs
        uint8 flags
        uint16 extra  # number of arg slots for local GC roots

    cdef struct JSIdArray:
        jsint length
        jsid  vector[1]  # actually, length jsid words

    cdef JSBool JSVAL_IS_BOOLEAN(jsval v)
    cdef JSBool JSVAL_IS_DOUBLE(jsval v)
    cdef JSBool JSVAL_IS_GCTHING(jsval v)
    cdef JSBool JSVAL_IS_INT(jsval v)
    cdef JSBool JSVAL_IS_NULL(jsval v)
    cdef JSBool JSVAL_IS_NUMBER(jsval v)
    cdef JSBool JSVAL_IS_OBJECT(jsval v)
    cdef JSBool JSVAL_IS_PRIMITIVE(jsval v)
    cdef JSBool JSVAL_IS_STRING(jsval v)
    cdef JSBool JSVAL_IS_VOID(jsval v)

    cdef JSBool JSVAL_TO_BOOLEAN(jsval v)
    cdef jsdouble* JSVAL_TO_DOUBLE(jsval v)
    cdef void* JSVAL_TO_GCTHING(jsval v)
    cdef int JSVAL_TO_INT(jsval v)
    cdef JSObject* JSVAL_TO_OBJECT(jsval v)
    cdef JSString* JSVAL_TO_STRING(jsval v)
    cdef void* JSVAL_TO_PRIVATE(jsval v)
    cdef JSString* JSVAL_TO_STRING(jsval v)

    cdef jsval BOOLEAN_TO_JSVAL(JSBool b)
    cdef jsval DOUBLE_TO_JSVAL(jsdouble* dp)
    cdef jsval INT_TO_JSVAL(int i)
    cdef jsval OBJECT_TO_JSVAL(JSObject* obj)    
    cdef jsval PRIVATE_TO_JSVAL(void* p)
    cdef jsval STRING_TO_JSVAL(JSString* str)

    cdef int JSVAL_TAG(jsval v)
    cdef void JSVAL_SETTAG(jsval v, int t)
    cdef void JSVAL_CLRTAG(jsval v)

    # Runtime Functions
    cdef JSRuntime* JS_NewRuntime(uint32 maxbytes)
    cdef void JS_DestroyRuntime(JSRuntime* rt)
    cdef JSContext* JS_ContextIterator(JSRuntime* rt, JSContext** iterp)

    # Context Functions
    cdef JSContext* JS_NewContext(JSRuntime* rt, size_t stackChunkSize)
    cdef void JS_DestroyContext(JSContext* cx)
    cdef JSRuntime* JS_GetRuntime(JSContext* cx)
    cdef JSObject* JS_GetGlobalObject(JSContext* cx)
    cdef void* JS_GetContextPrivate(JSContext* cx)
    cdef void JS_SetContextPrivate(JSContext* cx, void* data)
    
    # Type Functions
    cdef JSType JS_TypeOfValue(JSContext* cx, jsval v)

    # Class Functions
    cdef JSBool JS_InitStandardClasses(JSContext* cx, JSObject* obj)
    cdef JSObject* JS_GetClassObject(JSClass* klass)

    cdef JSObject* JS_InitClass(
        JSContext* cx,
        JSObject* obj,
        JSObject* parent_proto,
        JSClass* clasp,
        JSNative constructor,
        uintN nargs,
        JSPropertySpec* ps,
        JSFunctionSpec* fs,
        JSPropertySpec* static_ps,
        JSFunctionSpec* static_fs
    )

    cdef JSBool JS_PropertyStub(JSContext* cx, JSObject* obj, jsval id, jsval* vp)
    cdef JSBool JS_EnumerateStub(JSContext* cx, JSObject* obj)
    cdef JSBool JS_ResolveStub(JSContext* cx, JSObject* obj, jsval id)
    cdef JSBool JS_ConvertStub(JSContext* cx, JSObject* obj, JSType type, jsval* vp)
    cdef void JS_FinalizeStub(JSContext* cx, JSObject* obj)

    # Object Functions
    # Create an object as a property of another object.
    cdef JSObject* JS_DefineObject(
        JSContext* cx,
        JSObject* obj,
        char* name,
        JSClass* clasp,
        JSObject* proto,
        uintN attrs
    )
    
    # Create an object
    cdef JSObject* JS_NewObject(
        JSContext* cx,
        JSClass* clasp,
        JSObject* proto,
        JSObject* parent
    )
    
    cdef JSClass* JS_GetClass(JSContext* cx, JSObject* obj)
    cdef JSObject* JS_GetPrototype(JSContext* cx, JSObject* obj)
    cdef JSIdArray* JS_Enumerate(JSContext* cx, JSObject* obj)
    cdef void* JS_GetPrivate(JSContext* cx, JSObject* obj)
    cdef JSBool JS_SetPrivate(JSContext* cx, JSObject* obj, void* data)

    # Property Methods
    cdef JSBool JS_GetUCProperty(JSContext* cx, JSObject* obj, jschar* name, size_t slen, jsval* vp)
    cdef JSBool JS_SetUCProperty(JSContext* cx, JSObject* obj, jschar* name, size_t slen, jsval* vp)
    cdef JSBool JS_DeleteUCProperty2(JSContext* cx, JSObject* obj, jschar* name, size_t slen, jsval* vp)

    cdef JSBool JS_DefineUCProperty(
        JSContext* cx,
        JSObject* obj,
        jschar* name,
        size_t slen,
        jsval value,
        JSPropertyOp getter,
        JSPropertyOp setter,
        uintN attrs
    )

    # Array Functions
    cdef JSBool JS_IsArrayObject(JSContext* cx, JSObject* obj)
    cdef JSObject* JS_NewArrayObject(JSContext* cx, jsint length, jsval* vector)
    cdef JSBool JS_GetArrayLength(JSContext* cx, JSObject* obj, jsuint* lengthp)
    cdef JSBool JS_GetElement(JSContext* cx, JSObject* obj, jsint index, jsval* vp)
    cdef JSBool JS_SetElement(JSContext* cx, JSObject* obj, jsint index, jsval* vp)

    cdef JSBool JS_DefineElement(
        JSContext* cx,
        JSObject* obj,
        jsint index,
        jsval value,
        JSPropertyOp getter,
        JSPropertyOp setter,
        uintN attrs
    )

    # Function Functions
    cdef char* JS_GetFunctionName(JSFunction* fun)
    cdef JSBool JS_ObjectIsFunction(JSContext* cx, JSObject* obj)
    cdef JSObject* JS_GetFunctionObject(JSFunction* fun)
    cdef JSFunction* JS_ValueToFunction(JSContext* cx, jsval v)
    
    # Not necessarily specific to functions, but used for the undocumented
    # reserved slots.
    JSBool JS_GetReservedSlot(JSContext* cx, JSObject* obj, uint32 index, jsval* vp)
    JSBool JS_SetReservedSlot(JSContext* cx, JSObject* obj, uint32 index, jsval v)
    
    # Set a list of functions on an object
    cdef JSBool JS_DefineFunctions(JSContext* cx, JSObject* obj, JSFunctionSpec* fs)
    
    # Create a function property
    cdef JSFunction* JS_DefineUCFunction(
        JSContext* cx,
        JSObject* obj,
        char* name,
        JSNative call,
        uintN nargs,
        uintN attrs
    )
    
    # Create an anonymous function
    cdef JSFunction* JS_NewFunction(
        JSContext* cx,
        JSNative call,
        uintN nargs,
        uintN flags,
        JSObject* parent,
        char* name
    )
    
    # Call a named function
    cdef JSBool JS_CallFunctionName(
        JSContext* cx,
        JSObject* obj,
        char* name,
        uintN argc,
        jsval* argv,
        jsval* rval
    ) except *
    
    # Call a function by jsval reference
    cdef JSBool JS_CallFunctionValue(
        JSContext* cx,
        JSObject* obj,
        jsval fun,
        uintN argc,
        jsval* argv,
        jsval* rval
    ) except *

    # String Functions
    cdef JSString* JS_NewUCStringCopyN(JSContext* cx, jschar* s, size_t n)
    cdef char* JS_GetStringBytes(JSString* str)
    cdef jschar* JS_GetStringChars(JSString* str)
    cdef size_t JS_GetStringLength(JSString* str)

    # Double Functions
    cdef JSBool JS_NewNumberValue(JSContext* cx, jsdouble d, jsval* rval)

    # IdArray Functions
    cdef JSBool JS_IdToValue(JSContext* cx, jsid id, jsval* vp)
    cdef void JS_DestroyIdArray(JSContext* cx, JSIdArray* ida)

    # Memory Functions
    cdef void JS_GC(JSContext *cx)
    cdef void JS_MaybeGC(JSContext* cx)
    cdef void JS_free(JSContext* cx, void* p)
    cdef JSBool JS_AddRoot(JSContext* cx, void* rp)
    cdef JSBool JS_AddNamedRoot(JSContext* cx, void* rp, char* name)
    cdef JSBool JS_RemoveRoot(JSContext* cx, void* rp)

    # Error Reporting
    cdef JSBool JS_IsExceptionPending(JSContext* cx)
    cdef JSBool JS_GetPendingException(JSContext* cx, jsval* vp)
    cdef void JS_SetPendingException(JSContext* cx, jsval v)
    cdef void JS_ClearPendingException(JSContext* cx)
    
    cdef JSErrorReporter JS_SetErrorReporter(JSContext* cx, JSErrorReporter er)
    cdef void JS_ReportError(JSContext* cx, char* format, ...)

    # Main Entry Functions
    cdef JSBool JS_EvaluateUCScript(
        JSContext* cx,
        JSObject* obj,
        jschar* bytes,
        uintN length,
        char* filename,
        uintN lineno,
        jsval* rval
    )

cdef extern from "jshelpers.c":
    cdef JSClass js_global_class
    cdef JSObject* js_make_global_object(JSContext *cx)

    cdef void js_context_attach(JSContext* cx, PyObject* obj)
    cdef JSBool js_context_has_data(JSContext* cx)
    cdef object js_context_fetch(JSContext* cx)
    cdef object js_context_destroy(JSContext* cx)

    cdef void js_object_attach(JSContext* cx, JSObject* js_obj, PyObject* obj)
    cdef JSBool js_object_has_data(JSContext* cx, JSObject* js_obj)
    cdef object js_object_fetch(JSContext* cx, JSObject* js_obj)
    cdef object js_object_destroy(JSContext* cx, JSObject* js_obj)

    cdef void js_function_attach(JSContext* cx, JSObject* js_obj, PyObject* obj)
    cdef JSBool js_function_has_data(JSContext* cx, JSObject* js_obj)
    cdef object js_function_fetch(JSContext* cx, JSObject* js_obj)
    cdef object js_function_destroy(JSContext* cx, JSObject* js_obj)

    cdef JSString* py2js_jsstring_c(JSContext* cx, PyObject* str)
    cdef PyObject* js2py_jsstring_c(JSString* str)

cdef void* xmalloc(size_t size) except NULL:
    cdef void* mem
    mem = malloc(size)
    if <int>mem == 0:
        raise MemoryError()
    return mem

cdef class PyJSString:
    cdef JSString* data
    
    cdef jschar* chars(PyJSString self):
        return JS_GetStringChars(self.data)
    
    cdef size_t length(PyJSString self):
        return JS_GetStringLength(self.data)

cdef PyJSString py2js_jsstring(JSContext* cx, object str):
    cdef PyJSString ret
    cdef JSString* js_str
    js_str = py2js_jsstring_c(cx, <PyObject*> str)
    if js_str == NULL:
        raise UnicodeError("Failed to encode Python string as UTF-16")
    ret = PyJSString()
    ret.data = js_str
    return ret

cdef object js2py_jsstring(JSString* str):
    cdef PyObject* ret
    cdef object var
    if str == NULL:
        raise TypeError("Unable to convert a NULL JSString.")
    ret = js2py_jsstring_c(str)
    if ret != NULL:
        var = <object> ret
    return var

# JavaScript -> Python
cdef class Context
cdef class Function
cdef class GC
cdef class Runtime
cdef class Value

# Python -> JavaScript
cdef class ClassAdapter
cdef class ObjectAdapter
cdef class FunctionAdapter

def test_utf_16_round_trip(Context cx, data):
    cdef PyJSString conv
    conv = py2js_jsstring(cx.cx, data)
    return js2py_jsstring(conv.data)

import inspect
import sys
import traceback
import types

class JSError(Exception):
    def __init__(self, mesg):
        self.mesg = mesg
    def __str__(self):
        return repr(self)
    def __repr__(self):
        return self.mesg

