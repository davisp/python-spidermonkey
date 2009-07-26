/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_JAVASCRIPT_H
#define PYSM_JAVASCRIPT_H

extern JSClass js_global_class;

JSClass* create_class(Context* cx, PyObject* pyobj);
JSBool new_py_iter(Context* cx, PyObject* obj, jsval* rval);
void add_frame(const char* srcfile, const char* funcname, int linenum);
void report_error_cb(JSContext* cx, const char* message, JSErrorReport* report);

#endif