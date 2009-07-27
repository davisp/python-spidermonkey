/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#include <spidermonkey.h>
#include "frameobject.h"

JSBool
js_error(JSContext* cx, const char* err)
{
    JS_ReportError(cx, err);
    return JS_FALSE;
}

void
add_frame(const char* srcfile, const char* funcname, int linenum)
{
    PyObjectXDR src = PyString_FromString(srcfile);
    PyObjectXDR func = PyString_FromString(funcname);
    PyObjectXDR tpl = PyTuple_New(0);
    PyObjectXDR str = PyString_FromString("");

    // This is a borrowed reference, hence no PyObjectXDR so we
    // don't decref it.
    PyPtr<PyObject> glbl = PyModule_GetDict(SpidermonkeyModule);

    if(!src || !func || !glbl || !tpl || !str) return;

    PyCodeXDR code = PyCode_New(
        0,                      /*co_argcount*/
        0,                      /*co_nlocals*/
        0,                      /*co_stacksize*/
        0,                      /*co_flags*/
        str.get(),              /*co_code*/
        tpl.get(),              /*co_consts*/
        tpl.get(),              /*co_names*/
        tpl.get(),              /*co_varnames*/
        tpl.get(),              /*co_freevars*/
        tpl.get(),              /*co_cellvars*/
        src.get(),              /*co_filename*/
        func.get(),             /*co_name*/
        linenum,                /*co_firstlineno*/
        str.get()               /*co_lnotab*/
    );
    if(!code) return;
   
    PyFrameXDR frame = PyFrame_New(
        PyThreadState_Get(), code.get(), glbl.get(), NULL
    );
    if(!frame) return;
    
    frame->f_lineno = linenum;
    PyTraceBack_Here(frame.get());
}

void
report_error_cb(JSContext* cx, const char* message, JSErrorReport* report)
{
    const char* srcfile = report->filename;
    const char* mesg = message;

    if(!PyErr_Occurred())
    {
        PyErr_SetString(JSError, "Error executing JavaScript.");
    }

    if(srcfile == NULL) srcfile = "<JavaScript>";
    if(mesg == NULL) mesg = "<Unknown Error>";
    
    add_frame(srcfile, mesg, report->lineno);
}
