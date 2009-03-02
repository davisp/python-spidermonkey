#include "spidermonkey.h"

//static JSBool js_ctor(JSContext*, JSObject*, uintN, jsval*, jsval*);

JSBool 
js_add_prop(JSContext* cx, JSObject* obj, jsval key, jsval* val)
{
    return JS_TRUE;
}

JSBool
js_del_prop(JSContext* cx, JSObject* obj, jsval key, jsval* val)
{
    return JS_TRUE;
}

JSBool
js_get_prop(JSContext* cx, JSObject* obj, jsval key, jsval* val)
{
    return JS_TRUE;
}

JSBool
js_set_prop(JSContext* cx, JSObject* obj, jsval key, jsval* val)
{
    return JS_TRUE;
}

JSBool
js_enumerate(JSContext* cx, JSObject* obj, JSIterateOp op, jsval* st, jsid* id)
{
    return JS_TRUE;
}

JSBool
js_resolve(JSContext* cx, JSObject* obj, jsval key)
{
    return JS_TRUE;
}

void
js_finalize(JSContext* cx, JSObject* obj)
{
    return;
}

JSClass*
create_class(Context* cx, PyTypeObject* type)
{
    PyObject* curr = NULL;
    JSClass* jsclass = NULL;

    curr = Context_get_class(cx, type->tp_name);
    if(curr != NULL) return (JSClass*) PyCObject_AsVoidPtr(curr);

    jsclass = (JSClass*) malloc(sizeof(JSClass));
    if(jsclass == NULL)
    {
        fprintf(stderr, "No class!\n");
        PyErr_NoMemory();
        return NULL;
    }
   
    jsclass->name = (char*) malloc(strlen(type->tp_name)*sizeof(char));
    if(jsclass->name == NULL)
    {
        free(jsclass);
        PyErr_NoMemory();
        return NULL;
    }
    
    strcpy((char*) jsclass->name, type->tp_name);

    jsclass->flags = JSCLASS_HAS_RESERVED_SLOTS(2) | JSCLASS_NEW_ENUMERATE;
    jsclass->addProperty = js_add_prop;
    jsclass->delProperty = js_del_prop;
    jsclass->getProperty = js_get_prop;
    jsclass->setProperty = js_set_prop;
    jsclass->enumerate = (JSEnumerateOp) js_enumerate;
    jsclass->resolve = js_resolve;
    jsclass->convert = JS_ConvertStub;
    jsclass->finalize = js_finalize;
    jsclass->getObjectOps = NULL;
    jsclass->checkAccess = NULL;
    jsclass->call = NULL;
    jsclass->construct = NULL;
    jsclass->xdrObject = NULL;
    jsclass->hasInstance = NULL;
    jsclass->mark = NULL;
    jsclass->reserveSlots = NULL;
    
    curr = PyCObject_FromVoidPtr(jsclass, NULL);
    if(curr == NULL)
    {
        free((char*) jsclass->name);
        free(jsclass);
        return NULL;
    }

    if(Context_add_class(cx, type->tp_name, curr) < 0)
    {
        free((char*) jsclass->name);
        free(jsclass);
        return NULL;
    }

    return jsclass;
}

jsval
py2js_object(Context* cx, PyObject* pyobj)
{
    PyObject* hashable = NULL;
    JSClass* klass = NULL;
    JSObject* jsobj = NULL;
    jsval pyval;

    klass = create_class(cx, pyobj->ob_type);
    if(klass == NULL) return JSVAL_VOID;

    jsobj = JS_NewObject(cx->cx, klass, NULL, NULL);
    if(jsobj == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create JS object.");
        return JSVAL_VOID;
    }

    Py_INCREF(pyobj);
    pyval = PRIVATE_TO_JSVAL(pyobj);
    if(!JS_SetReservedSlot(cx->cx, jsobj, 0, pyval))
    {
        Py_DECREF(pyobj);
        PyErr_SetString(PyExc_RuntimeError, "Failed store ref'ed object.");
        return JSVAL_VOID;
    }

    hashable = PyCObject_FromVoidPtr(pyobj, NULL);
    if(hashable == NULL)
    {
        Py_DECREF(pyobj);
        PyErr_SetString(PyExc_RuntimeError, "Failed to make hashable pointer.");
        return JSVAL_VOID;
    }

    if(Context_add_object(cx, hashable) < 0)
    {
       Py_DECREF(pyobj);
       PyErr_SetString(PyExc_RuntimeError, "Failed to store object reference.");
       return JSVAL_VOID;
    }

    return OBJECT_TO_JSVAL(jsobj);
}


