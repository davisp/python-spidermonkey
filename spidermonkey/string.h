#ifndef PYSM_UTILS_H
#define PYSM_UTILS_H
 
JSString* py2js_string_obj(Context* cx, PyObject* str);
jsval py2js_string(Context* cx, PyObject* str);
PyObject* js2py_string(Context* cx, jsval val);

#endif
