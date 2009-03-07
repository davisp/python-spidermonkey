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
assert ret["foo"] == "bar"
ret["pinky"] = "the world"
assert cx.execute("f.pinky;") == "the world"
cx.execute("f[2] = 13;")
assert ret[2] == 13
assert ret["2"] == 13
del ret[2]
assert cx.execute('f["2"];') == None

ret = cx.execute('[1, "foo", undefined];');
assert repr(ret) == "1,foo,"
assert ret.length == 3

ret = cx.execute('function() {return "yipee";};')
assert ret() == "yipee"

ret = cx.execute("function(arg) {return arg * arg;};")
assert ret(4) == 16

# adding globals
cx.add_global("biz", 3)
assert cx.execute("biz * biz;") == 9

class Foo(object):
    def __init__(self):
        self.blam = 8

myglbl = Foo()
cx.add_global("rain", myglbl)
assert cx.execute("rain;") == myglbl
cx.execute("rain.blam = 8;");
assert myglbl.blam == 8
assert cx.execute("rain.blam / 2;") == 4
assert cx.execute("rain.blam = 3; rain.blam / 3;") == 1
assert hasattr(myglbl, "blam") == True
cx.execute("delete rain.blam;")
assert hasattr(myglbl, "blam") == False

def meander():
    return "Meandering enthusiastically!"
cx.add_global("meander", meander)
assert cx.execute("meander();").startswith("Meandering")
assert cx.execute('var f = {"foo": "bar"}; f;') == {"foo": "bar"}

def do_raise():
    raise RuntimeError()
cx.add_global("do_raise", do_raise)
cx.execute("\n\ndo_raise();")

