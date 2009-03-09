/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_ERROR_H
#define PYSM_ERROR_H

/*
    I'm dropping the second parameter. As it's
    only used so that we can see what function
    caused the call to the error frame.
*/
#define ERROR(f) add_frame(__FILE__, (f), __LINE__)
void add_frame(const char* srcfile, const char* funcname, int linenum);
void report_error_cb(JSContext* cx, const char* message, JSErrorReport* report);

#endif
