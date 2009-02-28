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
        traceback.print_exc()

import spidermonkey
r = spidermonkey.Runtime()
raises(spidermonkey.Context)
raises(spidermonkey.Context, "foo")
