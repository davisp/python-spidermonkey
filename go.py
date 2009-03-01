#! /usr/bin/env python

import glob
import sys
import traceback

libdirs = glob.glob("./build/lib*")
if len(libdirs) > 0:
    sys.path.insert(0, libdirs[0])

def raises(fn, *args, **kwargs):
    try:
        fn(*args, **kwargs)
    except:
        pass
    else:
        raise RuntimeError("Function did not throw.")

import spidermonkey
r = spidermonkey.Runtime()
raises(spidermonkey.Context)
raises(spidermonkey.Context, "foo")

cx = r.new_context()
assert cx.execute("var f = 4; f * f;") == 16
assert cx.execute("22/7;") - 3.14285714286 < 0.00000001

ret = cx.execute('var f = {"foo": "bar"}; f;')
assert repr(ret) == "[object Object]"
assert ret.foo == u"bar"
ret.pinky = "taking over."
assert cx.execute("f.pinky;") == u"taking over."

ret = cx.execute('[1, "foo", undefined];');
assert repr(ret) == "1,foo,"
assert ret.length == 3

ret = cx.execute('function() {return "yipee";};')
assert ret() == "yipee"

ret = cx.execute("function(arg) {return arg * arg;};")
assert ret(4) == 16
