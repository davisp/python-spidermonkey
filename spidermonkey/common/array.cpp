#include <spidermonkey.h>

PyObject*
js2py_array(Context* cx, jsval val)
{
    return make_object(ArrayType, cx, val);
}