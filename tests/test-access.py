# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
# Copyright 2009 Richard Boulton <richard@tartarus.org>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.cx()
def test_set_callback(cx):
    t.eq(cx.set_access(), None)
    t.raises(TypeError, cx.set_access, "foo")
    t.raises(TypeError, cx.set_access, 1)
    def cb(obj, name): return True
    t.eq(cx.set_access(cb), None)
    t.eq(cx.set_access(), cb)
    # Check that we don't erase it.
    t.eq(cx.set_access(), cb)

@t.cx()
def test_always_callback(cx):
    names = []
    def fn(obj, name):
        names.append(name)
        return True
    cx.set_access(fn)
    cx.add_global("foo", {"bar": "baz"})
    t.eq(cx.execute('foo["bar"]'), "baz")
    t.eq(names, ["bar"])

@t.cx()
def test_no_underscores(cx):
    def fn(obj, name):
        return name.strip()[:1] != "_"
    cx.set_access(fn)
    cx.add_global("foo", {"bar": "baz", "_bar": "_baz"})
    t.eq(cx.execute('foo["bar"]'), "baz")
    t.raises(t.JSError, cx.execute, 'foo["_bar"]')

@t.cx()
def test_no_set_invalid(cx):
    def fn(obj, name):
        return name.strip()[:1] != "_"
    cx.set_access(fn)
    glbl = {}
    cx.add_global("foo", glbl)
    cx.execute('foo["bar"] = "baz";')
    t.raises(t.JSError, cx.execute, 'foo["_bar"] = "baz"')
    t.eq(glbl, {"bar": "baz"})

@t.rt()
def test_access_in_ctor(rt):
    def fn(obj, name):
        return name != "bing"
    cx = rt.new_context(access=fn)
    cx.add_global("foo", {"bing": "boo"})
    t.raises(t.JSError, cx.execute, 'foo["bing"];')

@t.rt()
def test_access_global(rt):
    names = []
    def fn(obj, name):
        names.append(name)
        return not name.startswith("_")
    glbl = {"foo": "bar", "_bin": "zingle"}
    cx = rt.new_context(glbl, fn)
    t.eq(cx.execute('foo;'), "bar")
    t.raises(t.JSError, cx.execute, '_bin;')
    t.eq(names, ["foo", "foo", "_bin"])

@t.cx()
def test_dissallow_ctor(cx):
    class DirtyCar(object):
        def __init__(self):
            self.zap = 2
    def check(obj, name):
        return name != "__init__"
    cx.add_global("DirtyCar", DirtyCar)
    cx.set_access(check)
    t.raises(t.JSError, cx.execute, "DirtyCall();")

@t.cx()
def test_dissalow_call(cx):
    class PepsiCan(object):
        def __init__(self):
            self.caffeine = "zaney!"
        def __call__(self, arg):
            return arg * 2
    def check(obj, name):
        return name != "__call__"
    cx.add_global("PepsiCan", PepsiCan)
    cx.set_access(check)
    t.eq(cx.execute("var c = new PepsiCan(); c.caffeine;"), "zaney!")
    t.raises(t.JSError, cx.execute, "c();")

@t.cx()
def test_on_wrapped_obj(cx):
    class ShamWow(object):
        def __init__(self):
            self.bar = 2
            self._bing = 3
    def func():
        return ShamWow()
    cx.add_global("whee", func)

    def check(obj, name):
        return name in ["__call__", "__init__"] or not name.startswith("_")
    cx.set_access(check);

    t.eq(cx.execute("var f = whee(); f.bar;"), 2)
    t.raises(t.JSError, cx.execute, "f._bing")

@t.cx()
def test_obj_method(cx):
    class Checker(object):
        def __init__(self):
            self.names = []
        def check(self, obj, name):
            self.names.append(name)
            return name != "kablooie"
    c = Checker()
    cx.set_access(c.check)
    cx.add_global("bing", {"kablooie": "bar", "bing": 3})
    t.eq(cx.execute('bing["bing"]'), 3)
    t.raises(t.JSError, cx.execute, 'bing["kablooie"]')
    t.eq(c.names, ["bing", "kablooie"])
