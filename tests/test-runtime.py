# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.rt()
def test_creating_runtime(rt):
    t.ne(rt, None)

def test_create_no_memory():
    rt = t.spidermonkey.Runtime(1)
    t.raises(RuntimeError, rt.new_context)

def test_exceed_memory():
    # This test actually tests nothing. I'm leaving it for a bit to
    # see if I hear about the bug noted below.
    rt = t.spidermonkey.Runtime(50000)
    cx = rt.new_context()
    script = "var b = []; var f = 1000; while(f-- > 0) b.push(2.456);"
    # I had this script below original and it triggers some sort of
    # bug in the JS VM. I even reduced the test case outside of
    # python-spidermonkey to show it. No word from the SM guys.
    # script = "var b = []; for(var f in 100000) b.push(2.456);"
    cx.execute(script)

