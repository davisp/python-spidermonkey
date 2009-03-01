#ifndef PYSM_CONVERT_H
#define PYSM_CONVERT_H

jsval py2js(Context* cx, PyObject* obj);
PyObject* js2py(Context* cx, jsval val);

#endif
