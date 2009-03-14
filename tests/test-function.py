# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t
import threading

@t.cx()
def test_call_js_func(cx):
    t.eq(cx.execute('function() {return "yipee";};')(), "yipee")

@t.cx()
def test_call_js_with_arg(cx):
    func = cx.execute("function(v) {return v * 2;};")
    t.eq(func(2), 4)

@t.cx()
def test_function_with_dict_arg(cx):
    func = cx.execute("function(doc) {if(doc.data) return doc.data;};")
    t.eq(func({"data": 2}), 2)
    t.eq(func({}), None)

@t.cx()
def test_function_returning_unicode(cx):
    def bar():
        return u"\u0042"
    cx.add_global("bound", bar)
    t.eq(cx.execute("bound();"), u"\u0042")

@t.cx()
def test_global_function(cx):
    def meander():
        return "Meandering enthusiastically!"
    cx.add_global("meander", meander)
    t.eq(cx.execute("meander();"), "Meandering enthusiastically!")

@t.cx()
def test_bound_method(cx):
    class Foo(object):
        def callme(self, arg):
            return "I am serious. And don't call me Shirley."
    f = Foo()
    cx.add_global("func", f.callme)
    ret = cx.execute('func("Surely you can\'t be serious.");')
    t.eq("Shirley" in ret, True)

@t.cx()
def test_bound_method_no_return(cx):
    class Foo(object):
        def callme(self):
            pass
    f = Foo()
    cx.add_global("func", f.callme)
    ret = cx.execute("func()")
    t.eq(ret, None)

@t.cx()
def test_bound_method_from_js_func(cx):
    class Foo(object):
        def callme(self):
            return 4
    f = Foo()
    cx.add_global("dude", f.callme)
    func = cx.execute("function(a) {return a * dude();}")
    ret = func(3)
    t.eq(ret, 12)

def test_bound_method_in_thread():
    class Foo(object):
        def __init__(self):
            self.t = threading.Thread(target=self.run)
            self.t.start()
        def run(self):
            rt = t.spidermonkey.Runtime()
            cx = rt.new_context()
            cx.add_global("stuff", self.call)
            ret = cx.execute("stuff() * 4;")
            t.eq(ret, 8)
        def call(self):
            return 2
    f = Foo()
    f.t.join()
            
            
