/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include "spidermonkey.h"

JSString*
py2js_string_obj(Context* cx, PyObject* str)
{
    PyObject* conv = NULL;
    PyObject* encoded = NULL;
    JSString* ret = NULL;
    char* bytes;
    Py_ssize_t len;

    if(PyString_Check(str))
    {
        conv = PyUnicode_FromEncodedObject(str, "utf-8", "replace");
        if(conv == NULL) goto error;
        str = conv;
    }
    else if(!PyUnicode_Check(str))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid string conversion.");
        goto error;
    }

    encoded = PyUnicode_AsEncodedString(str, "utf-16", "strict");
    if(encoded == NULL) goto error;
    if(PyString_AsStringAndSize(encoded, &bytes, &len) < 0) goto error;
    if(len < 2)
    {
        PyErr_SetString(PyExc_ValueError, "Failed to find byte-order mark.");
        goto error;
    }

    if(((unsigned short*) bytes)[0] != 0xFEFF)
    {
        PyErr_SetString(PyExc_ValueError, "Invalid UTF-16 BOM");
        goto error;
    }

    ret = JS_NewUCStringCopyN(cx->cx, (jschar*) (bytes+2), (len/2)-1);
    
    goto success;

error:
success:
    Py_XDECREF(conv);
    Py_XDECREF(encoded);
    return ret;
}

jsval
py2js_string(Context* cx, PyObject* str)
{
    JSString* val = py2js_string_obj(cx, str);
    if(val == NULL)
    {
        PyErr_Clear();
        return JSVAL_VOID;
    }

    return STRING_TO_JSVAL(val);
}

PyObject*
js2py_string(Context* cx, jsval val)
{
    JSString* str;
    jschar* bytes;
    size_t len;

    if(!JSVAL_IS_STRING(val))
    {
        PyErr_SetString(PyExc_TypeError, "Value is not a JS String.");
        return NULL;
    }

    str = JSVAL_TO_STRING(val);
    len = JS_GetStringLength(str);
    bytes = JS_GetStringChars(str);

    return PyUnicode_Decode((const char*) bytes, len*2, "utf-16", "strict");
}
