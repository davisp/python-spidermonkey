# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.rt()
def test_iter_py(rt):
    pairs = [
        ({"foo": "bar", "baz": "bam"}, ["foo", "baz"]),
        (["a", "b", "c"], [0, 1, 2])
    ]
    def check(a, b):
        cx = rt.new_context()
        cx.add_global("zip", a)
        js = """
            var ret = [];
            for(var v in zip) {ret.push(v);}
            ret;
        """
        t.eq(cx.execute(js), b)
    map(lambda x: check(*x), pairs)

@t.rt()
def test_iter_js(rt):
    pairs = [
        ('var f = {"foo": 1, "domino": "daily"}; f;', ["domino", "foo"]),
        ('["foo", 1, "bing"]', [0, 1, 2])
    ]
    def check(a, b):
        cx = rt.new_context()
        ret = cx.execute(a)
        data = [k for k in ret]
        data.sort()
        t.eq(data, b)
    map(lambda x: check(*x), pairs)
