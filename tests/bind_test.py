import unittest
import spidermonkey

class BindTest(unittest.TestCase):
    def setUp(self):
        class Nonce(object): pass
        class Window(object):
            def __init__(self):
                self.arg = Nonce()
                self.window = self
                self.name = "foobar"
                self.val = 42
            def foo(self, arg):
                self.arg = arg
                return "hi %s" % arg
        self.window = Window()
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context(self.window)
        self.cx.install_class(Nonce)

    def bind_attribute(self):
        class foo:
            def __init__(self):
                self.bar = 1
        f = foo()
        self.cx.bind("spam", f)
        self.assertEqual(self.cx.execute("spam;"), f.bar)

    def test_bind_global(self):
        self.assertEqual(self.cx.execute('name;'), self.window.name)
        self.assertEqual(self.cx.execute('arg;'), self.window.arg)
        self.assertEqual(self.cx.execute('window;'), self.window)
        self.assertEqual(self.cx.execute('window.arg;'), self.window.arg)
        self.assertEqual(self.cx.execute('foo(12);'), "hi 12")
        self.assertEqual(self.cx.execute('arg;'), 12)
        self.cx.execute('var spam = 13;')
        self.assertEqual(self.cx.execute('spam;'), 13)
        self.cx.execute('window.arg = 14;')
        self.assertEqual(self.cx.execute('window.arg;'), 14)
        self.assertEqual(self.window.arg, 14)
    
    def test_referenced(self):
        def bind(cx):
            cx.bind("data", object())
        self.assertRaises(spidermonkey.JSError, self.cx.execute, "data;")
        bind(self.cx)
        self.assertNotEqual(self.cx.execute("data;"), None)
        self.assertEqual(self.cx.execute("data;"), self.cx.unbind("data"))
        self.assertRaises(spidermonkey.JSError, self.cx.execute, "data;")