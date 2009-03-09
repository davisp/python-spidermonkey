# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

touched = 0
class Foo(object):
    def __init__(self):
        self.bar = 2
    def __del__(self):
        global touched
        touched = 1

@t.glbl("Foo", Foo)
def test_py_ctor_right_type(cx, glbl):
    t.eq(isinstance(cx.execute("var f = new Foo(); f;"), Foo), True)

@t.glbl("Foo", Foo)
def test_py_ctor_attribute_acc(cx, glbl):
    t.eq(cx.execute("var f = new Foo(); f;").bar, 2)

@t.glbl("Foo", Foo)
def test_py_dtor_called(cx, glbl):
    t.eq(cx.execute('var f = {"baz": new Foo()}; f;').baz.bar, 2)
    cx.execute("delete f.baz;")
    cx.gc()
    t.eq(touched, 1)
