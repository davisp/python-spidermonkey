
#include "spidermonkey.h"

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

PyTypeObject* RuntimeType = NULL;
PyTypeObject* ContextType = NULL;
PyTypeObject* ClassType = NULL;
PyTypeObject* ObjectType = NULL;

static PyMethodDef spidermonkey_methods[] = {
    {NULL}
};

PyMODINIT_FUNC
initspidermonkey(void)
{
    PyObject* m;
    
    if(PyType_Ready(&_RuntimeType) < 0) return;
    if(PyType_Ready(&_ContextType) < 0) return;
    if(PyType_Ready(&_ClassType) < 0) return;
    if(PyType_Ready(&_ObjectType) < 0) return;
    
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

    ClassType = &_ClassType;
    Py_INCREF(ClassType);
    PyModule_AddObject(m, "Class", (PyObject*) ClassType);

    ObjectType = &_ObjectType;
    Py_INCREF(ObjectType);
    PyModule_AddObject(m, "Object", (PyObject*) ObjectType);
}
