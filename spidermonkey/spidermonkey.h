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

#ifdef __cplusplus
#include "utils/utils.h"
#endif

#include "convert/convert.h"
#include "javascript/javascript.h"
#include "python/python.h"

extern PyObject* SpidermonkeyModule;
extern PyObject* JSError;

#endif
