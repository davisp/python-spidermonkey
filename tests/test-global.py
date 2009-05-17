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

@t.rt()
def test_py_with_global(rt):
    rt.new_context({})

@t.rt()
def test_py_with_invalid_global(rt):
    t.raises(TypeError, rt.new_context, "Break!")

@t.rt()
def test_py_get_global(rt):
    glbl = {"foo": "bar"}
    cx = rt.new_context(glbl)
    t.eq(cx.execute("foo;"), "bar")

@t.rt()
def test_py_set_global(rt):
    glbl = {}
    cx = rt.new_context(glbl)
    cx.execute("foo = 71;")
    t.eq(cx.execute("foo;"), 71);
    t.eq(glbl["foo"], 71)

class FunctionTest(object):
    def __init__(self):
        self.data = {}
    def __getitem__(self, key):
        return self.data[key]
    def __setitem__(self, key, value):
        self.data[key] = value

@t.rt()
def test_py_set_function_global(rt):
    glbl = FunctionTest()
    cx = rt.new_context(glbl)
    cx.execute("function foo() {};")
    t.is_js_object(glbl["foo"])

class ActiveGlobal(object):
    def __init__(self):
        self.data = {}
    def __getitem__(self, key):
        return self.data[key]
    def __setitem__(self, key, value):
        self.data[key] = value * 2

@t.rt()
def test_py_no_del_item(rt):
    glbl = ActiveGlobal()
    cx = rt.new_context(glbl)
    cx.execute('foo = 4;')
    t.eq(glbl.data["foo"], 8)
    cx.execute("delete foo;")
    t.isin("foo", glbl.data)

class ActiveGlobalWithDel(ActiveGlobal):
    def __delitem__(self, key):
        del self.data[key]

@t.rt()
def test_py_del_global(rt):
    glbl = ActiveGlobalWithDel()
    cx = rt.new_context(glbl)
    cx.execute("foo = 4;")
    t.eq(glbl.data["foo"], 8)
    cx.execute("delete foo;")
    t.isnotin("foo", glbl.data)

@t.rt()
def test_py_with_active_global(rt):
    glbl = ActiveGlobal()
    cx = rt.new_context(glbl)
    cx.execute("foo = 4;")
    t.eq(cx.execute("foo;"), 8)
    t.eq(glbl.data["foo"], 8);
