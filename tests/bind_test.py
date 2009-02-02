import unittest
import spidermonkey

class BindTest(unittest.TestCase):
    def setUp(self):
        class Nonce(object): pass
        class Window(object):
            def __init__(self):
                self.arg = Nonce()
                self.window = self
                self.name = u"foobar"
                self.val = 42
            def foo(self, arg):
                self.arg = arg
                return u"hi %s" % arg
        self.window = Window()
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context(self.window)
        self.cx.install_class(Nonce)

    def test_bind_attribute(self):
        class foo:
            def __init__(self):
                self.bar = 1
        f = foo()
        self.cx.bind(u"spam", f)
        self.assertEqual(self.cx.execute(u"spam;").bar, f.bar)

    def test_bind_function(self):
        def func():
            return u"foo"
        self.cx.bind(u"func", func)
        self.assertEqual(self.cx.execute(u'func();'), u"foo")

    def test_bind_global(self):
        self.assertEqual(self.cx.execute(u'name;'), self.window.name)
        self.assertEqual(self.cx.execute(u'arg;'), self.window.arg)
        self.assertEqual(self.cx.execute(u'window;'), self.window)
        self.assertEqual(self.cx.execute(u'window.arg;'), self.window.arg)
        self.assertEqual(self.cx.execute(u'foo(12);'), u"hi 12")
        self.assertEqual(self.cx.execute(u'arg;'), 12)
        self.cx.execute(u'var spam = 13;')
        self.assertEqual(self.cx.execute(u'spam;'), 13)
        self.cx.execute(u'window.arg = 14;')
        self.assertEqual(self.cx.execute(u'window.arg;'), 14)
        self.assertEqual(self.window.arg, 14)
    
    def test_referenced(self):
        def bind(cx):
            cx.bind(u"data", object())
        self.assertRaises(spidermonkey.JSError, self.cx.execute, u"data;")
        bind(self.cx)
        self.assertNotEqual(self.cx.execute(u"data;"), None)
        self.assertEqual(self.cx.execute(u"data;"), self.cx.unbind(u"data"))
        self.assertRaises(spidermonkey.JSError, self.cx.execute, u"data;")
        
if __name__ == "__main__":
    unittest.main()
