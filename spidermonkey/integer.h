#ifndef PYSM_INTEGER_H
#define PYSM_INTEGER_H

jsval py2js_integer(Context* cx, PyObject* obj);
PyObject* js2py_integer(Context* cx, jsval val);

#endif
