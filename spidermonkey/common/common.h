/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_COMMON_H
#define PYSM_COMMON_H

#include "python/python.h"

jsval py2js(Context* cx, PyObject* obj);
PyObject* js2py(Context* cx, jsval val);
PyObject* js2py_with_parent(Context* cx, jsval val, jsval parent);

jsval py2js_double(Context* cx, PyObject* obj);
PyObject* js2py_double(Context* cx, jsval val);

jsval py2js_integer(Context* cx, PyObject* obj);
jsval long2js_integer(Context* cx, long val);
PyObject* js2py_integer(Context* cx, jsval val);

JSString* py2js_string_obj(Context* cx, PyObject* str);
jsval py2js_string(Context* cx, PyObject* str);
PyObject* js2py_string(Context* cx, jsval val);

jsval py2js_object(Context* cx, PyObject* pyobj);
PyObject* js2py_object(Context* cx, jsval val);

PyObject* js2py_function(Context* cx, jsval val, jsval parent);

#endif
