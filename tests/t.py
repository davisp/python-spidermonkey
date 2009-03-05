import unittest
import spidermonkey

class test(object):
    def __init__(self, lbl):
        self.lbl = lbl
    def __call__(self, func):
        def run():
            func(*self.args())
        run.func_name = "test %s" % self.lbl
        return run
    def args(self, func):
        raise NotImplementedError()

class rt(test):
    def args(self):
        return (spidermonkey.Runtime(),)

class cx(test):
    def args(self):
        rt = spidermonkey.Runtime()
        return (rt.new_context(),)

class echo(test):
    def args(self):
        rt = spidermonkey.Runtime()
        cx = rt.new_context()
        echo = cx.execute("var echo = function(foo) {return foo;}; echo;")
        return (echo,)

def eq(a, b):
    assert a == b, "%r != %r" % (a, b)

def ne(a, b):
    assert a != b, "%r == %r" % (a, b)

def raises(exctype, func, *args, **kwargs):
    try:
        func(*args, **kwargs)
    except exctype, inst:
        pass
    else:
        raise AssertionError("Function %s did not raise %s" % (
            func.func_name, exctype.__name__))

