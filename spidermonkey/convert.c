/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

jsval
py2js(Context* cx, PyObject* obj)
{
    if(obj == Py_None)
    {
        return JSVAL_NULL;
    }
    else if(obj == Py_True)
    {
        return JSVAL_TRUE;
    }
    else if(obj == Py_False)
    {
        return JSVAL_FALSE;
    }
    else if(PyInt_Check(obj) || PyLong_Check(obj))
    {
        return py2js_integer(cx, obj);
    }
    else if(PyFloat_Check(obj))
    {
        return py2js_double(cx, obj);
    }
    else if(PyString_Check(obj) || PyUnicode_Check(obj))
    {
        return py2js_string(cx, obj);
    }
    else if(PyObject_TypeCheck(obj, ObjectType))
    {
        return ((Object*) obj)->val;
    }
    else
    {
        return py2js_object(cx, obj);
    }

    PyErr_SetString(PyExc_ValueError, "Unable to convert Python value to JS.");
    return JSVAL_VOID;
}

PyObject*
js2py(Context* cx, jsval val)
{
    return js2py_with_parent(cx, val, JSVAL_NULL);
}

PyObject*
js2py_with_parent(Context* cx, jsval val, jsval parent)
{
    JSType vtype = JS_TypeOfValue(cx->cx, val);

    /*
        There's not JSType for null. Or rather, its
        reported as Object which causes segfaults.
    */
    if(val == JSVAL_NULL || val == JSVAL_VOID)
    {
        Py_RETURN_NONE;
    }
    else if(vtype == JSTYPE_BOOLEAN)
    {
        if(val == JSVAL_TRUE)
        {
            Py_RETURN_TRUE;
        }
        else
        {
            Py_RETURN_FALSE;
        }
    }
    else if(vtype == JSTYPE_STRING)
    {
        return js2py_string(cx, val);
    }
    else if(vtype == JSTYPE_NUMBER)
    {
        if(JSVAL_IS_INT(val)) return js2py_integer(cx, val);
        else return js2py_double(cx, val);
    }
    else if(vtype == JSTYPE_FUNCTION)
    {
        return js2py_function(cx, val, parent);
    }
    else if(vtype == JSTYPE_OBJECT)
    {
        JSObject* obj = JSVAL_TO_OBJECT(val);
        if(JS_IsArrayObject(cx->cx, obj))
        {
            return js2py_array(cx, val);
        }
        
        return js2py_object(cx, val);
    }
    
    PyErr_SetString(PyExc_RuntimeError, "Unknown JSVAL type.");
    return NULL;
}
