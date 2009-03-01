#ifndef PYSM_JSOBJECT_H
#define PYSM_JSOBJECT_H

/*
    This is a representation of a JavaScript
    object in Python land.
*/

typedef struct {
    PyObject_HEAD
    Context* cx;
    jsval val;
    JSObject* obj;
} Object;

extern PyTypeObject _ObjectType;

PyObject* make_object(PyTypeObject* type, Context* cx, jsval val);
PyObject* js2py_object(Context* cx, jsval val);

#endif
