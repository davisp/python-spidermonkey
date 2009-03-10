/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_DOUBLE_H
#define PYSM_DOUBLE_H

jsval py2js_double(Context* cx, PyObject* obj);
PyObject* js2py_double(Context* cx, jsval val);

#endif
