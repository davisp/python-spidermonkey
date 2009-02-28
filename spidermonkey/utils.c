#include "spidermonkey.h"

JSString*
py2js_string(JSContext* cx, PyObject* str)
{
    JSString* rval = NULL;
    PyObject* conv = NULL;
    PyObject* encoded = NULL;
    char* bytes;
    Py_ssize_t len;

    if(PyString_Check(str))
    {
        conv = PyUnicode_FromEncodedObject(str, "utf-8", "replace");
        if(conv == NULL) return NULL;
        str = conv;
    }
    else if(!PyUnicode_Check(str))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid string conversion.");
        return NULL;
    }

    encoded = PyUnicode_AsEncodedString(str, "utf-16", "strict");
    if(encoded == NULL) goto cleanup;
    if(PyString_AsStringAndSize(encoded, &bytes, &len) < 0) goto cleanup;
    if(len < 4) goto cleanup;

    rval = JS_NewUCStringCopyN(cx, (jschar*) (bytes+2), (len/2)-1);
    
cleanup:
    Py_XDECREF(conv);
    return rval;
}

PyObject*
js2py_string(JSString* str)
{
    jschar* bytes;
    size_t len;

    if(str == NULL)
    {
        PyErr_SetString(PyExc_TypeError, "Unable to convert NULL JSString");
        return NULL;
    }

    len = JS_GetStringLength(str);
    bytes = JS_GetStringChars(str);

    return PyUnicode_Decode((const char*) bytes, len*2, "utf-16", "strict");
}
