#include "spidermonkey.h"
    
void
report_error_cb(JSContext* cx, char* message, JSErrorReport* report)
{
    fprintf(stderr, "Error: %s\n", message);
}
