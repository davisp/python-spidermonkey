# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.cx()
def test_to_py_int(cx):
    t.eq(cx.execute("42;"), 42)

@t.cx()
def test_to_py_float(cx):
    t.eq(cx.execute("42.5;"), 42.5)

@t.cx()
def test_to_py_str(cx):
    t.eq(cx.execute('"spam";'), "spam")
    t.eq(isinstance(cx.execute('"spam";'), unicode), True)

@t.cx()
def test_to_py_unicode(cx):
    t.eq(cx.execute(u"\"\u0042\";"), u"\u0042")

@t.cx()
def test_undefined_to_py_None(cx):
    t.eq(cx.execute("undefined;"), None)

@t.cx()
def test_null_to_py_None(cx):
    t.eq(cx.execute("null;"), None)

@t.cx()
def test_true_to_py_True(cx):
    t.eq(cx.execute("true;"), True)

@t.cx()
def test_to_py_False(cx):
    t.eq(cx.execute("false;"), False)

@t.cx()
def test_NaN_to_py_nan(cx):
    nan = cx.execute("NaN;")
    t.eq(type(nan), float)
    t.ne(nan, nan)

@t.cx()
def test_Infinity_to_py_inf(cx):
    t.eq(cx.execute("Infinity;"), 1E500*1E500)
