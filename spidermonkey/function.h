#ifndef PYSM_FUNCTION_H
#define PYSM_FUNCTION_H

jsval py2js_function(Context* cx, PyObject* obj);
PyObject* js2py_function(Context* cx, jsval val);

#endif
