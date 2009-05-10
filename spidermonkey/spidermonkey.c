/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

PyObject* SpidermonkeyModule = NULL;
PyTypeObject* RuntimeType = NULL;
PyTypeObject* ContextType = NULL;
PyTypeObject* ObjectType = NULL;
PyTypeObject* ArrayType = NULL;
PyTypeObject* FunctionType = NULL;
PyTypeObject* IteratorType = NULL;
PyTypeObject* HashCObjType = NULL;
PyObject* JSError = NULL;

static PyMethodDef spidermonkey_methods[] = {
    {NULL}
};

PyMODINIT_FUNC
initspidermonkey(void)
{
    PyObject* m;
    
    if(PyType_Ready(&_RuntimeType) < 0) return;
    if(PyType_Ready(&_ContextType) < 0) return;
    if(PyType_Ready(&_ObjectType) < 0) return;

    _ArrayType.tp_base = &_ObjectType;
    if(PyType_Ready(&_ArrayType) < 0) return;

    _FunctionType.tp_base = &_ObjectType;
    if(PyType_Ready(&_FunctionType) < 0) return;

    if(PyType_Ready(&_IteratorType) < 0) return;

    if(PyType_Ready(&_HashCObjType) < 0) return;
    
    m = Py_InitModule3("spidermonkey", spidermonkey_methods,
            "The Python-Spidermonkey bridge.");

    if(m == NULL)
    {
        return;
    }

    RuntimeType = &_RuntimeType;
    Py_INCREF(RuntimeType);
    PyModule_AddObject(m, "Runtime", (PyObject*) RuntimeType);

    ContextType = &_ContextType;
    Py_INCREF(ContextType);
    PyModule_AddObject(m, "Context", (PyObject*) ContextType);

    ObjectType = &_ObjectType;
    Py_INCREF(ObjectType);
    PyModule_AddObject(m, "Object", (PyObject*) ObjectType);

    ArrayType = &_ArrayType;
    Py_INCREF(ArrayType);
    PyModule_AddObject(m, "Array", (PyObject*) ArrayType);

    FunctionType = &_FunctionType;
    Py_INCREF(FunctionType);
    PyModule_AddObject(m, "Function", (PyObject*) FunctionType);

    IteratorType = &_IteratorType;
    Py_INCREF(IteratorType);
    // No module access on purpose.

    HashCObjType = &_HashCObjType;
    Py_INCREF(HashCObjType);
    // Don't add access from the module on purpose.

    JSError = PyErr_NewException("spidermonkey.JSError", NULL, NULL);
    PyModule_AddObject(m, "JSError", JSError);
    
    SpidermonkeyModule = m;
}
