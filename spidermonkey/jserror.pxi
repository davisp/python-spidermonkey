

cdef JSBool report_python_error(JSContext* cx):
    tb = traceback.print_exc()
    JS_ReportError(cx, tb)
    return JS_FALSE

cdef void __report_error_callback__(JSContext* cx, char* message, JSErrorReport* report):
    cdef Context pycx
    pycx = js_context_fetch(cx)

    lines = report.linebuf
    lines = lines.split("\n")

    if len(lines) == 0:
        msg = ["%s:\n<no code>" % message]
    else:
        i = report.lineno
        n = 2  # lines of context on either side of where error occurred

        start, end = max(i-n, 0), min(i+n+1, len(lines))

        msg = []
        for ln in lines[start:i]:
            msg.append("   %s" % ln)
        msg.append("-> %s\n" % lines[i])
        for ln in lines[i+1:end]:
            msg.append("   %s" % ln)
        msg.append("")
        msg.append('JavaScript error at line %d: "%s"' % (report.lineno, message))
    context.set_error("\n".join(mesg))

