/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_PYTHON_H
#define PYSM_PYTHON_H

extern PyTypeObject _ArrayType;
extern PyTypeObject _ContextType;
extern PyTypeObject _FunctionType;
extern PyTypeObject _HashCObjType;
extern PyTypeObject _IteratorType;
extern PyTypeObject _ObjectType;
extern PyTypeObject _RuntimeType;

typedef struct {
    PyObject_HEAD
    JSRuntime* rt;
} Runtime;

typedef struct {
    PyObject_HEAD
    Runtime* rt;
    JSContext* cx;
    PyObject* pyglobal;
    JSObject* jsglobal;
    PyObject* access;
    PyDictObject* classes;
    PySetObject* objects;
} Context;

typedef struct {
    PyObject_HEAD
    Context* cx;
    jsval val;
    JSObject* obj;
} Object;

typedef struct {
    Object obj;
    jsval parent;
} Function;

typedef struct {
    PyObject_HEAD
    void* cobj;
} HashCObj;

typedef struct {
    PyObject_HEAD
    Context* cx;
    JSObject* iter;
    jsval root;
} Iterator;

// Context
int Context_has_access(Context*, JSContext*, PyObject*, PyObject*);
int Context_add_class(Context* cx, const char* key, PyObject* val);
PyObject* Context_get_class(Context* cx, const char* key);
int Context_has_object(Context* cx, PyObject* val);
int Context_add_object(Context* cx, PyObject* val);
int Context_rem_object(Context* cx, PyObject* val);

// HashCObj
// BIG FUCKING NOTE: This constructor never Py_INCREF's the returned object.
PyObject* HashCObj_FromVoidPtr(void *cobj);
void* HashCObj_AsVoidPtr(PyObject* self);

// Iterators
PyObject* Iterator_Wrap(Context* cx, JSObject* obj);

// Objects
PyObject* make_object(PyTypeObject* type, Context* cx, jsval val);

#endif

