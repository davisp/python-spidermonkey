#ifndef PYSM_PYOBJECT_H
#define PYSM_PYOBJECT_H

/*
    This represents a Python object in the
    JavaScript VM.
*/

jsval py2js_object(Context* cx, PyObject* obj);

#endif
