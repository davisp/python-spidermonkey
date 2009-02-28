#ifndef SPIDERMONKEY_H
#define SPIDERMONKEY_H

#include <Python.h>
#include "structmember.h"

#include "libjs/jsapi.h"

#include "runtime.h"
#include "context.h"
#include "class.h"

#include "utils.h"

extern PyTypeObject* RuntimeType;
extern PyTypeObject* ContextType;
extern PyTypeObject* ClassType;

#endif
