# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

class Foo(object):
    def __init__(self):
        self.blam = 8
        self.zing = "yessiree"

@t.glbl("zip", Foo())
def test_iter_py(cx, glbl):
    js = """
        var ret = [];
        for(var v in zip) {
            ret.push(v)
        }
        ret;
    """
    t.eq(cx.execute(js), ["blam", "zing"])

@t.cx()
def test_iter_js(cx):
    ret = cx.execute('var f = {"foo": 1, "domino": "daily"}; f;')
    keys = [k for k in ret]
    keys.sort()
    t.eq(keys, ["domino", "foo"])

