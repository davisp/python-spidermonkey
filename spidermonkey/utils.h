#ifndef PYSM_UTILS_H
#define PYSM_UTILS_H
    
JSString* py2js_string(JSContext* cx, PyObject* str);
PyObject* js2py_string(JSString* str);

#endif
