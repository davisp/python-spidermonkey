#ifndef PYSM_ERROR_H
#define PYSM_ERROR_H

/*
    I'm dropping the second parameter. As it's
    only used so that we can see what function
    caused the call to the error frame.
*/
#define ERROR(f, s) add_frame(__FILE__, (f), (s), __LINE__)
void add_frame(
        const char* srcfile, const char* funcname,
        const char* srcline, int linenum);
void report_error_cb(JSContext* cx, const char* message, JSErrorReport* report);

#endif
