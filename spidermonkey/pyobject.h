/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_PYOBJECT_H
#define PYSM_PYOBJECT_H

/*
    This represents a Python object in the
    JavaScript VM.
*/

jsval py2js_object(Context* cx, PyObject* obj);

#endif
