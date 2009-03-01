#ifndef PYSM_ERROR_H
#define PYSM_ERROR_H

void report_error_cb(JSContext* cx, const char* message, JSErrorReport* report);

#endif
