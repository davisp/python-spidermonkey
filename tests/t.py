import unittest
import spidermonkey

class rt(object):
    def __init__(self, lbl):
        self.lbl = lbl
    def __call__(self, func):
        def t():
            rt = spidermonkey.Runtime()
            func(rt)
        t.func_name = "test %s" % self.lbl
        return t

class cx(object):
    def __init__(self, lbl):
        self.lbl = lbl
    def __call__(self, func):
        def t():
            rt = spidermonkey.Runtime()
            cx = rt.new_context()
            func(cx)
        t.func_name = "test %s" % self.lbl
        return t

def eq(a, b):
    assert a == b, "%r != %r" % (a, b)

def neq(a, b):
    assert a != b, "%r == %r" % (a, b)
