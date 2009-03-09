# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.rt()
def test_no_provided_runtime(rt):
    t.raises(TypeError, t.spidermonkey.Context)

@t.rt()
def test_invalid_runtime(rt):
    t.raises(TypeError, t.spidermonkey.Context, 0)

@t.rt()
def test_creating_new_context(rt):
    t.eq(isinstance(rt.new_context(), t.spidermonkey.Context), True)

@t.cx()
def test_basic_execution(cx):
    t.eq(cx.execute("var x = 4; x * x;"), 16)
    t.lt(cx.execute("22/7;") - 3.14285714286, 0.00000001)
    

@t.cx()
def test_reentry(cx):
    cx.execute("var x = 42;")
    t.eq(cx.execute("x;"), 42)
