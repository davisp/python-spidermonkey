#include "runtime.h"

PyObject*
Runtime_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    Runtime* self;

    self = (Runtime*) type->tp_alloc(type, 0);
    if(self != NULL)
    {
        self->rt = JS_NewRuntime(1000000);
        if(self->rt == NULL)
        {
            Py_DECREF(self);
            return NULL;
        }
    }

    return (PyObject*) self;
}

void
Runtime_dealloc(Runtime* self)
{
    if(self->rt != NULL)
    {
        JS_DestroyRuntime(self->rt);
    }
}

int
Runtime_init(Runtime* self, PyObject* args, PyObject* kwargs)
{
    return 0;
}

