#ifndef PYSM_DOUBLE_H
#define PYSM_DOUBLE_H

jsval py2js_double(Context* cx, PyObject* obj);
PyObject* js2py_double(Context* cx, jsval val);

#endif
