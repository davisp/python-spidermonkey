# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.cx()
def test_bind_global(cx):
    cx.add_global("foo", "yay spring!")
    t.eq(cx.execute("foo;"), "yay spring!")

@t.cx()
def test_use_global(cx):
    cx.add_global("biz", 3)
    t.eq(cx.execute("biz * biz;"), 9)

@t.cx()
def test_unicode_in_global(cx):
    cx.add_global(u"\u00FC", 234)
    t.eq(cx.execute(u"\u00FC;"), 234)

@t.cx()
def test_unbind_global(cx):
    cx.add_global("bizzle", 29)
    t.eq(cx.execute("bizzle"), 29)
    t.eq(cx.rem_global("bizzle"), 29)
    t.raises(t.JSError, cx.execute, "bizzle")

@t.cx()
def test_unbinding_unicode(cx):
    cx.add_global(u"\u00E9", 734)
    t.eq(cx.execute(u"\u00E9;"), 734)
    t.eq(cx.rem_global(u"\u00E9"), 734)
    t.raises(t.JSError, cx.execute, u"\u00E9")

class Foo(object):
    def __init__(self):
        self.blam = 8

@t.glbl("rain", Foo())
def test_attr_fetch(cx, glbl):
    t.eq(cx.execute("rain.blam;"), 8)

@t.glbl("rain", Foo())
def test_attr_mutate(cx, glbl):
    cx.execute("rain.blam = 4;")
    t.eq(glbl.blam, 4)

@t.glbl("rain", Foo())
def test_attr_usage(cx, glbl):
    t.eq(cx.execute("rain.blam / 2;"), 4)

@t.glbl("rain", Foo())
def test_repeated_usage(cx, glbl):
    t.eq(cx.execute("rain.blam = 3; rain.blam / 3;"), 1)

@t.glbl("rain", Foo())
def test_js_rem_attr(cx, glbl):
    t.has(glbl, "blam")
    cx.execute("delete rain.blam;")
    t.hasnot(glbl, "blam")
