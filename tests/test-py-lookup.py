# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.cx()
def test_str_property_as_item(cx):
    cx.add_global("zim", {"gir": "tacito!"})
    t.eq(cx.execute('zim["gir"]'), "tacito!")

@t.cx()
def test_str_property_as_attr(cx):
    cx.add_global("protein", {"rna": "dna"})
    t.eq(cx.execute("protein.rna;"), "dna")

@t.cx()
def test_unicode_key(cx):
    cx.add_global("unicode", {u"is": "complicated"})
    t.eq(cx.execute("unicode.is;"), "complicated")

@t.cx()
def test_int_property(cx):
    cx.add_global("foo", [1, 8])
    t.eq(cx.execute("foo[1];"), 8)


# JavaScript property looks can only be integers and
# strings. So even though foo[1.1] looks like it should
# work, Spidermonkey is converting it to a string which
# affects access in python land.

@t.cx()
def test_float_prop(cx):
    cx.add_global("foo", {1.1: "hidden!"})
    t.eq(cx.execute("foo[1.1];"), None)

@t.cx()
def test_float_expected(cx):
    cx.add_global("whee", {"3.14": "mmmm food"})
    t.eq(cx.execute("whee[3.14];"), "mmmm food")
