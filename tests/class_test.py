import types
import unittest
import spidermonkey

class ClassTests(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context()
        class spam(object):
            def __init__(self):
                self.args = []
                self.val = 42
                self._private = u"no peeking"
            def foo(self, *args):
                self.args.append(args)
            def _private_method(self):
                assert False
            def __getitem__(self, key):
                assert isinstance(key, (types.IntType, types.LongType))
                self.args.append(key)
                return self.val
            def __setitem__(self, key, value):
                assert isinstance(key, (types.IntType, types.LongType))
                self.args.append((key, value))
                self.val = value
        self.cx.install_class(spam)
        self.spam = spam()
        self.cx.bind(u"bs", self.spam)

    def test_private(self):
        self.assertEqual(self.cx.execute(u"bs._private;"), None)
        self.cx.execute(u"bs._private = 1;")
        self.assertEqual(self.spam._private, u"no peeking")
        self.assertEqual(self.cx.execute(u"bs._private_method;"), None)
        self.assertEqual(self.cx.execute(u"bs._private_method = 1;"), True)
        self.assertEqual(isinstance(self.spam._private_method, types.MethodType), True)
        # in fact, even normal methods shouldn't be assignable to
        self.assertEqual(self.cx.execute(u"bs.foo = 1;"), True)
        self.assertEqual(isinstance(self.spam.foo, types.MethodType), True)

    def test_bind_module(self):
        self.assertRaises(TypeError, self.cx.install_class, __builtins__)

    def test_no_constructor(self):
        class foo:
            pass
        self.cx.install_class(foo, bind_constructor=False)
        self.assertRaises(spidermonkey.JSError, self.cx.execute, u"var f = new foo();")
        f2 = foo()
        self.cx.bind(u"f2", f2)
        self.assertEqual(self.cx.execute(u"f2;"), f2)

    def test_js_specific(self):
        class bar:
            __jsname__ = u"eggs"
            def __init__(self, arg):
                self.arg = arg
            
            @staticmethod
            def __jsinit__(cx):
                return bar(42)
        self.cx.install_class(bar)
        self.cx.execute(u"var e = new eggs();")
        self.assertEqual(self.cx.execute(u"e.arg;"), 42)

    def test_assign_new_property(self):
        self.cx.execute(u"bs.xyzzy = 1;")
        self.assertEqual(self.cx.execute(u"bs.xyzzy;"), 1)

    def test_identity(self):
        self.assertEqual(self.cx.execute(u"bs;"), self.spam)

    def test_call(self):
        self.cx.execute(u'bs.foo("hi");')
        self.assertEqual(self.spam.args.pop(), (u"hi",))

    def test_construct(self):
        s = self.cx.execute(u"""\
            var s = new spam();
            s.foo(1, "blah", ["1", 2, "three"]);
            s;
        """)
        self.assertEqual(s.args.pop(), (1, u"blah", [u"1", 2, u"three"]))

    def test_getsetitem(self):
        # property lookup with an integer is mapped to __get/__setitem__
        self.assertEqual(self.cx.execute(u"bs[0];"), 42)
        self.cx.execute(u"bs[0] = 2;")
        self.assertEqual(self.cx.execute(u"bs[0];"), 2)
        
if __name__ == "__main__":
    unittest.main()
