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

#include "utils/utils.h"
#include "convert/convert.h"
#include "javascript/javascript.h"
#include "python/python.h"

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
