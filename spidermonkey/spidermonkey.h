#ifndef SPIDERMONKEY_H
#define SPIDERMONKEY_H

#include <Python.h>
#include "structmember.h"

#include "libjs/jsapi.h"

#include "runtime.h"
#include "context.h"
#include "class.h"

#include "string.h"
#include "integer.h"
#include "double.h"
#include "function.h"

#include "jsobject.h"
#include "jsarray.h"
#include "jsfunction.h"

#include "convert.h"
#include "error.h"

extern PyTypeObject* RuntimeType;
extern PyTypeObject* ContextType;
extern PyTypeObject* ClassType;
extern PyTypeObject* ObjectType;
extern PyTypeObject* ArrayType;
extern PyTypeObject* FunctionType;

#endif
