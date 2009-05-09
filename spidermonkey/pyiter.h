/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_PYITER_H
#define PYSM_PYITER_H

/*
    This is a bit of glue between Python and JavaScript
    iterators.
*/

JSBool new_py_iter(Context* cx, PyObject* obj, jsval* rval);

#endif
