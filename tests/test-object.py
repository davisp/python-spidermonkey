# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.cx()
def test_object_repr(cx):
    t.eq(repr(cx.execute('var f = {"foo": "bar"}; f;')), "[object Object]")

@t.cx()
def test_object_attr_access(cx):
    t.eq(cx.execute('var f = {"foo": "bar"}; f;').foo, "bar")

@t.cx()
def test_object_item_access(cx):
    t.eq(cx.execute('var f = {"foo": "bar"}; f;')["foo"], "bar")

@t.cx()
def test_attribute_creation(cx):
    ret = cx.execute('var f = {"foo": "bar"}; f;')
    ret.pinky = "taking over."
    t.eq(cx.execute("f.pinky;"), "taking over.")

@t.cx()
def test_item_creation(cx):
    ret = cx.execute('var f = {"foo": "bar"}; f;')
    ret["pinky"] = "the world"
    t.eq(cx.execute("f.pinky;"), "the world")

@t.cx()
def test_js_mutation(cx):
    ret = cx.execute('var f = {"foo": "bar"}; f;')
    cx.execute('f["foo"] = 13;')
    t.eq(ret.foo, 13)

@t.cx()
def test_int_is_str_item(cx):
    ret = cx.execute('var f = {"2": "bar"}; f;')
    t.eq(ret["2"], "bar")
    t.eq(ret[2], "bar")

@t.cx()
def test_del_item_from_py(cx):
    ret = cx.execute('var f = {2: "bar"}; f;')
    del ret[2]
    t.eq(cx.execute('f[2];'), None)

@t.cx()
def test_del_attr_from_py(cx):
    ret = cx.execute('var f = {2: "bar"}; f;')
    delattr(ret, "2")
    t.eq(cx.execute("f[2]"), None)

@t.cx()
def test_array_repr(cx):
    t.eq(repr(cx.execute('[1, "foo", undefined];')), "1,foo,")

@t.cx()
def test_array_length(cx):
    t.eq(cx.execute('[1, "foo", undefined];').length, 3)

@t.cx()
def test_array_equality(cx):
    t.eq(cx.execute("[1,2,3];"), [1, 2, 3])

@t.cx()
def test_mapping_equality(cx):
    js = 'var d = {0: 0, "a": 1, 2: "b", "c": "d", "blah": 2.5}; d;'
    py = {0: 0, "a": 1, 2: "b", "c": "d", "blah": 2.5}
    t.eq(cx.execute(js), py)

@t.cx()
def test_nested_object_equality(cx):
    t.eq(
        cx.execute('["foo", 2, {"bar": 2.3, "spam": [1,2,3]}];'),
        [u"foo", 2, {u"bar": 2.3, u"spam": [1,2,3]}]
    )
