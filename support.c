#include <stdio.h>

#include "jsapi.h"

void
fail(char *msg) {fprintf(stderr, msg);};

static JSBool
Print_jsval(JSContext *cx, jsval obj)
{
    JSString *str;
    str = JS_ValueToString(cx, obj);
    if (!str)
        return JS_FALSE;
    fprintf(stdout, "%s\n", JS_GetStringBytes(str));
    return JS_TRUE;
}

static JSBool
Print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    uintN i, n;
    JSString *str;

    for (i = n = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return JS_FALSE;
        fprintf(stdout, "%s%s", i ? " " : "", JS_GetStringBytes(str));
    }
    n++;
    if (n)
        fputc('\n', stdout);
    return JS_TRUE;
}

static
JSFunctionSpec functions[] = {
    {"print", Print, 0},
    {0}
};

void
call_fn(JSContext *cx, JSObject *globalObj){
    JSString *input;
    jsval argv[1];
    jsval rval;
    int ok;
    input = JS_NewStringCopyN(cx, "Hello, world!", 14);
    argv[0] = STRING_TO_JSVAL(input);
    ok = JS_CallFunctionName(cx, globalObj, "print", 1, argv, &rval);
}

static JSClass global_class = {
    "MyClass", 0,

    JS_PropertyStub, // JSPropertyOp addProperty;
    JS_PropertyStub, // JSPropertyOp delProperty;
    JS_PropertyStub, // JSPropertyOp getProperty;
    JS_PropertyStub, // JSPropertyOp setProperty;
    JS_EnumerateStub, // JSEnumerateOp enumerate;
    JS_ResolveStub, // JSResolveOp resolve;
    JS_ConvertStub, // JSConvertOp convert;
    JS_FinalizeStub // JSFinalizeOp finalize;

    /* Optionally non-null members start here. */
    // JSGetObjectOps getObjectOps;
    // JSCheckAccessOp checkAccess;
    // JSNative call;
    // JSNative construct;
    // JSXDRObjectOp xdrObject;
    // JSHasInstanceOp hasInstance;
    // JSMarkOp mark;
    // jsword spare;
};

JSObject *
make_global_object(JSContext *cx) {
    /* Statically initialize a class to make "one-off" objects. */
    return JS_NewObject(cx, &global_class, 0, 0);
}
