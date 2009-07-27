
#include <spidermonkey.h>

JSRequest::JSRequest(JSContext* cx)
{
    this->cx = cx;
    JS_BeginRequest(cx);
}

JSRequest::~JSRequest()
{
    JS_EndRequest(cx);
}