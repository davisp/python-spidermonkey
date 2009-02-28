import sys
import traceback
sys.path.insert(0, "./build/lib.macosx-10.4-i386-2.5/")

def raises(fn, *args, **kwargs):
    try:
        fn(*args, **kwargs)
    except:
        traceback.print_exc()

import spidermonkey
r = spidermonkey.Runtime()
raises(spidermonkey.Context)
raises(spidermonkey.Context, "foo")
