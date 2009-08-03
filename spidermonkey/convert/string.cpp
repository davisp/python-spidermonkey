/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include <spidermonkey.h>

JSString*
py2js_string_obj(Context* cx, PyObject* str)
{
    PyObjectXDR conv;
    if(PyString_Check(str))
    {
        conv.set(PyUnicode_FromEncodedObject(str, "utf-8", "replace"));
        if(!conv) return NULL;
        str = conv.get();
    }
    else if(!PyUnicode_Check(str))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid string conversion.");
        return NULL;
    }

    PyObjectXDR enc = PyUnicode_AsEncodedString(str, "utf-16", "strict");
    if(!enc) return NULL;

    char* bytes;
    Py_ssize_t len;
    if(PyString_AsStringAndSize(enc.get(), &bytes, &len) < 0)return NULL;

    if(len < 2)
    {
        PyErr_SetString(PyExc_ValueError, "Failed to find byte-order mark.");
        return NULL;
    }

    if(((unsigned short*) bytes)[0] != 0xFEFF)
    {
        PyErr_SetString(PyExc_ValueError, "Invalid UTF-16 BOM");
        return NULL;
    }

    return JS_NewUCStringCopyN(cx->cx, (jschar*) (bytes+2), (len/2)-1);
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
