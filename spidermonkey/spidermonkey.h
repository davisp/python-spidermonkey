/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef SPIDERMONKEY_H
#define SPIDERMONKEY_H

#include <Python.h>
#include "structmember.h"

#include <jsapi.h>

#include "runtime.h"
#include "context.h"

#include "string.h"
#include "integer.h"
#include "double.h"

#include "pyobject.h"
#include "pyiter.h"

#include "jsobject.h"
#include "jsarray.h"
#include "jsfunction.h"
#include "jsiterator.h"

#include "convert.h"
#include "error.h"

#include "hashcobj.h"

extern PyObject* SpidermonkeyModule;
extern PyTypeObject* RuntimeType;
extern PyTypeObject* ContextType;
extern PyTypeObject* ClassType;
extern PyTypeObject* ObjectType;
extern PyTypeObject* ArrayType;
extern PyTypeObject* FunctionType;
extern PyTypeObject* IteratorType;
extern PyTypeObject* HashCObjType;
extern PyObject* JSError;

#endif
