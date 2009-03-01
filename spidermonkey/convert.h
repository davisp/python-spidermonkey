#ifndef PYSM_CONVERT_H
#define PYSM_CONVERT_H

jsval py2js(Context* cx, PyObject* obj);
PyObject* js2py(Context* cx, jsval val);
PyObject* js2py_with_parent(Context* cx, jsval val, jsval parent);

#endif
