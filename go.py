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
print repr(ret.foo)
