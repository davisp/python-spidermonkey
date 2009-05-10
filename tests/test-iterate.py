# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

js_for_script = """
var ret = [];
for(var v in data) {ret.push(v);}
ret;
"""

js_for_each_script = """
var ret = [];
for each(var v in data) {ret.push(v);}
ret;
"""

@t.glbl("data", {"foo": "bar", "baz": "bam"})
def test_iter_py_map(cx, glbl):
    t.eq(cx.execute(js_for_script), ["foo", "baz"])
    t.eq(cx.execute(js_for_each_script), ["bar", "bam"])

@t.glbl("data", ["a", 2, "zing!"])
def test_iter_py_array(cx, glbl):
    t.eq(cx.execute(js_for_script), [0, 1, 2])
    t.eq(cx.execute(js_for_each_script), ["a", 2, "zing!"])

@t.cx()
def test_iter_js_object(cx):
    ret = cx.execute('var f = {"foo": 1, "domino": "daily"}; f;')
    items = set(["domino", "foo"])
    for k in ret:
        t.isin(k, items)
        items.remove(k)

@t.cx()
def test_iter_js_array(cx):
    ret = cx.execute('["foo", 1, "bing", [3, 6]]')
    t.eq([k for k in ret], ["foo", 1, "bing", [3, 6]])
