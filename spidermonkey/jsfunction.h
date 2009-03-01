#ifndef PYSM_JSFUNCTION_H
#define PYSM_JSFUNCTION_H

/*
    This is a representation of a JavaScript
    Function in Python land.
*/

typedef struct {
    Object obj;
    jsval parent;
} Function;

extern PyTypeObject _FunctionType;

PyObject* js2py_function(Context* cx, jsval val, jsval parent);

#endif
