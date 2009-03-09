# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import unittest
import spidermonkey
from spidermonkey import JSError

class test(object):
    def __call__(self, func):
        def run():
            func(*self.args())
        run.func_name = func.func_name
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
        echo = cx.execute("function(arg) {return arg;}")
        return (echo,)

class glbl(test):
    def __init__(self, name, value):
        self.name = name
        self.value = value
    def args(self):
        rt = spidermonkey.Runtime()
        cx = rt.new_context()
        cx.add_global(self.name, self.value)
        return (cx, self.value)        

def eq(a, b):
    assert a == b, "%r != %r" % (a, b)

def ne(a, b):
    assert a != b, "%r == %r" % (a, b)

def lt(a, b):
    assert a < b, "%r >= %r" % (a, b)

def gt(a, b):
    assert a > b, "%r <= %r" % (a, b)

def has(a, b):
    assert hasattr(a, b), "%r has no attribute %r" % (a, b)

def hasnot(a, b):
    assert not hasattr(a, b), "%r has an attribute %r" % (a, b)

def raises(exctype, func, *args, **kwargs):
    try:
        func(*args, **kwargs)
    except exctype, inst:
        pass
    else:
        raise AssertionError("Function %s did not raise %s" % (
            func.func_name, exctype.__name__))

