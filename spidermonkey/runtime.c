#include "runtime.h"
#include "context.h"

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

int
Runtime_init(Runtime* self, PyObject* args, PyObject* kwargs)
{
    return 0;
}

void
Runtime_dealloc(Runtime* self)
{
    if(self->rt != NULL)
    {
        JS_DestroyRuntime(self->rt);
    }
}

PyObject*
Runtime_new_context(Runtime* self, PyObject* args, PyObject* kwargs)
{
    Context* cx = NULL;
    PyObject* root = NULL;
    PyObject* a = NULL;
    PyObject* k = NULL;
    
    static char *kwlist[] = {"root", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", kwlist, &root))
    {
        return NULL;
    }
    
    a = Py_BuildValue("(O)", self);
    if(a == NULL) return NULL;

    if(root != NULL)
    {
        k = Py_BuildValue("{s:O}", "root", root);
        if(k == NULL) return NULL;
    }
    else
    {
        k = NULL;
    }
   
    cx = Context_new(&ContextType, a, k);
    
    Py_XDECREF(a);
    Py_XDECREF(k);

    return cx;
}
