

cdef JSBool report_python_error(JSContext* cx):
    tb = traceback.format_exc()
    JS_ReportError(cx, tb)
    return JS_FALSE

cdef void __report_error_callback__(JSContext* cx, char* message, JSErrorReport* report):
    cdef Context pycx
    
    try:
        if not js_context_has_data(cx):
            sys.stderr.write("Attempting to report error for an unknown JSContext.\n")
            return
        pycx = js_context_fetch(cx)
        
        filename = "Unknown"
        lineno = "Unknown"
        msg = "Unknown"
        
        if report != NULL:
            if report.filename != NULL:
                filename = report.filename
            lineno = report.lineno
        if message != NULL:
            msg = message
        
        pycx.set_error("%s(%d): %s" % (filename, lineno, message))
    except:
        traceback.print_exc()
