/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_JSITERATOR_H
#define PYSM_JSITERATOR_H

/*
    This is a representation of a JavaScript
    object in Python land.
*/

typedef struct {
    PyObject_HEAD
    Context* cx;
    JSObject* iter;
    jsval root;
} Iterator;

extern PyTypeObject _IteratorType;

PyObject* Iterator_Wrap(Context* cx, JSObject* obj);

#endif
