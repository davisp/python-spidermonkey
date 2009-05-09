/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_INTEGER_H
#define PYSM_INTEGER_H

jsval py2js_integer(Context* cx, PyObject* obj);
jsval long2js_integer(Context* cx, long val);
PyObject* js2py_integer(Context* cx, jsval val);

#endif
