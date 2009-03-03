#ifndef PYSM_HASHCOBJ_H
#define PYSM_HASHCOBJ_H

/*
    A class to implement Python hashing of C pointers.
*/

typedef struct {
    PyObject_HEAD
    void* cobj;
} HashCObj;

extern PyTypeObject _HashCObjType;


/*
    BIG FUCKING NOTE: This constructor never
    Py_INCREF's the returned object.
*/
PyObject* HashCObj_FromVoidPtr(void *cobj);
void* HashCObj_AsVoidPtr(PyObject* self);

#endif
