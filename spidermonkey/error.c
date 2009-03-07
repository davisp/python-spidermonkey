#include "spidermonkey.h"
#include "frameobject.h" // Python
#include "traceback.h" // Python

void
add_frame(const char* srcfile, const char* funcname,
            const char* srcline, int linenum)
{
    PyObject* src = NULL;
    PyObject* func = NULL;
    PyObject* srcln = NULL;
    PyObject* glbl = NULL;
    PyObject* tpl = NULL;
    PyObject* str = NULL;
    PyCodeObject* code = NULL;
    PyFrameObject* frame = NULL;

    src = PyString_FromString(srcfile);
    if(src == NULL) goto error;

    func = PyString_FromString(funcname);
    if(func == NULL) goto error;

    if(srcline != NULL) srcln = PyString_FromString(srcline);
    else srcln = PyString_FromString("");
    if(srcln == NULL) goto error;
    
    glbl = PyModule_GetDict(SpidermonkeyModule);
    if(glbl == NULL) goto error;

    tpl = PyTuple_New(0);
    if(tpl == NULL) goto error;

    str = PyString_FromString("");
    if(str == NULL) goto error;

    code = PyCode_New(
        0,                      /*co_argcount*/
        0,                      /*co_nlocals*/
        0,                      /*co_stacksize*/
        0,                      /*co_flags*/
        srcln,                  /*co_code*/
        tpl,                    /*co_consts*/
        tpl,                    /*co_names*/
        tpl,                    /*co_varnames*/
        tpl,                    /*co_freevars*/
        tpl,                    /*co_cellvars*/
        src,                    /*co_filename*/
        func,                   /*co_name*/
        linenum,                /*co_firstlineno*/
        str                     /*co_lnotab*/
    );
    if(code == NULL) goto error;
    
    frame = PyFrame_New(PyThreadState_Get(), code, glbl, NULL);
    if(frame == NULL) goto error;
    
    frame->f_lineno = linenum;
    PyTraceBack_Here(frame);

    goto success;
    
error:
success:
    Py_XDECREF(func);
    Py_XDECREF(src);
    Py_XDECREF(tpl);
    Py_XDECREF(str);
    Py_XDECREF(code);
    Py_XDECREF(frame);
}

void
report_error_cb(JSContext* cx, const char* message, JSErrorReport* report)
{
    //If the root error is from Python, don't add a message.
    if(PyErr_Occurred())
    {
        add_frame(report->filename, message, report->linebuf, report->lineno);
    }
    else
    {
        fprintf(stderr, "Error: %s\n", message);
    }
}
