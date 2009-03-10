/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_STRING_H
#define PYSM_STRING_H
 
JSString* py2js_string_obj(Context* cx, PyObject* str);
jsval py2js_string(Context* cx, PyObject* str);
PyObject* js2py_string(Context* cx, jsval val);

#endif
