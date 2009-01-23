import unittest
from unittest import TestCase

from types import IntType, MethodType

from spidermonkey import Runtime, JSError

# XXX call_fn

class context_tests(TestCase):
    def setUp(self):
        rt = Runtime()
        self.cx = rt.new_context()

    def test_scope(self):
        # multiple evaluations in a Context share same scope
        cx = self.cx
        cx.eval_script("var x = 42;")
        self.assert_(cx.eval_script("x;") == 42)

class test_conversions_to_Python(TestCase):
    def setUp(self):
        rt = Runtime()
        self.cx = rt.new_context()
    def test_primitive_types(self):
        self.assert_(self.cx.eval_script("42;") == 42)
        self.assert_(self.cx.eval_script("42.5;") == 42.5)
        self.assert_(self.cx.eval_script('"spam";') == "spam")
        self.assert_(self.cx.eval_script("undefined;") is None)
        self.assert_(self.cx.eval_script("null;") is None)
        self.assert_(self.cx.eval_script("true;") is True)
        self.assert_(self.cx.eval_script("false;") is False)
        #self.assert_(self.cx.eval_script("Infinity;") is XXX)
        #self.assert_(self.cx.eval_script("NaN;") is XXX)

    def test_container_types(self):
        self.assert_(self.cx.eval_script("[1,2,3];") == [1,2,3])
        self.assert_(self.cx.eval_script(
            'var d = {0: 0, "a": 1, 2: "b", "c": "d", "blah": 2.5}; d;') ==
                     {0: 0, "a": 1, 2: "b", "c": "d", "blah": 2.5})
        self.assert_(self.cx.eval_script(
            '["foo", 2, {"bar": 2.3, "spam": [1,2,3]}];') ==
                     ["foo", 2, {"bar": 2.3, "spam": [1,2,3]}])

class test_conversions_two_way(TestCase):
    def setUp(self):
        rt = Runtime()
        self.cx = rt.new_context()
        self.x = []
        def echo(arg):
            self.x.append(arg)
            return arg
        self.cx.bind_callable("echo", echo)

    def check(self, script, arg):
        self.assert_(self.cx.eval_script(script) == arg)
        self.assert_(self.x.pop() == arg)

    def test_primitive_types(self):
        self.check("echo(42);", 42)
        self.check("echo(42.5);", 42.5)
        self.check('echo("spam");', "spam")
        self.check("echo(undefined);", None)
        self.check("echo(null);", None)
        self.check("echo(true);", True)
        self.check("echo(false);", False)
        #self.check("echo(Infinity);", XXX)
        #self.check("echo(NaN);", XXX)

class test_global(TestCase):
    def setUp(self):
        rt = Runtime()
        self.cx = rt.new_context()

    def test_get_global(self):
        self.cx.eval_script("foo = 1; bar = undefined; baz = null; bing = 2;")
        self.cx.get_global("foo")
        # XXXX are these two correct?
        self.assertRaises(ValueError, self.cx.get_global, ("bar"))
        self.assertRaises(ValueError, self.cx.get_global, ("baz"))
        self.cx.get_global("bing")
        self.assertRaises(ValueError, self.cx.get_global, ("spam"))

    def bind_attribute(self):
        class foo:
            def __init__(self): self.bar = 1
        f = foo()
        self.cx.bind_attribute("spam", f, "bar")
        self.assert_(self.cx.eval_script("spam;") == f.bar)
        self.assert_(self.cx.get_global("spam") == f.bar)

class test_class(TestCase):
    def setUp(self):
        rt = Runtime()
        self.cx = rt.new_context()
        class spam(object):
            def __init__(self):
                self.args = []
                self.val = 42
                self._private = "no peeking"
            def foo(self, *args):
                self.args.append(args)
            def _private_method(self): assert False
            def __getitem__(self, key):
                assert type(key) == IntType
                self.args.append(key)
                return self.val
            def __setitem__(self, key, value):
                assert type(key) == IntType
                self.args.append((key, value))
                self.val = value
        self.cx.bind_class(spam)
        self.spam = spam()
        self.cx.bind_object("bs", self.spam)

    def test_private(self):
        self.assert_(self.cx.eval_script("bs._private;") is None)
        self.cx.eval_script("bs._private = 1;")
        self.assert_(self.spam._private == "no peeking")
        self.assert_(self.cx.eval_script("bs._private_method;") is None)
        self.assert_(self.cx.eval_script("bs._private_method = 1;"))
        self.assert_(isinstance(self.spam._private_method, MethodType))
        # in fact, even normal methods shouldn't be assignable to
        self.assert_(self.cx.eval_script("bs.foo = 1;"))
        self.assert_(isinstance(self.spam.foo, MethodType))

    def test_bind_module(self):
        self.assertRaises(TypeError, self.cx.bind_class, __builtins__)

    def test_no_constructor(self):
        class foo: pass
        self.cx.bind_class(foo, bind_constructor=False)
        self.assertRaises(JSError,
                          self.cx.eval_script, "var f = new foo();")
        f2 = foo()
        self.cx.bind_object("f2", f2)
        self.assert_(self.cx.get_global("f2") is f2)

    def test_js_name_js_constructor(self):
        def constructor(cx):
            return bar(42)
        class bar:
            js_name = "eggs"
            js_constructor = (constructor,)
            def __init__(self, arg): self.arg = arg
        self.cx.bind_class(bar)
        self.cx.eval_script("var e = new eggs();")
        e = self.cx.get_global("e")
        self.assert_(e.arg == 42)

    def test_assign_new_property(self):
        self.cx.eval_script("bs.xyzzy = 1;")
        self.assert_(self.cx.eval_script("bs.xyzzy;") == 1)

    def test_identity(self):
        self.assert_(self.spam is self.cx.eval_script("bs;"))

    def test_call(self):
        self.cx.eval_script('bs.foo("hi");')
        self.assert_(self.spam.args.pop() == ("hi",))

    def test_construct(self):
        s = self.cx.eval_script("""\
            var s = new spam();
            s.foo(1, "blah", ["1", 2, "three"]);
            s;
        """)
        self.assert_(s.args.pop() == (1, "blah", ["1", 2, "three"]))

    def test_getsetitem(self):
        # property lookup with an integer is mapped to __get/__setitem__
        assert self.cx.eval_script("bs[0];") == 42
        self.cx.eval_script("bs[0] = 2;")
        assert self.cx.eval_script("bs[0];") == 2

class test_bind_global(TestCase):
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
        self.window = Window()
        rt = Runtime()
        self.cx = rt.new_context(self.window)
        self.cx.bind_class(Nonce)

    def test_bind_global(self):
        self.cx.eval_script('name;')
        self.cx.eval_script('window;')
        self.assert_(self.cx.eval_script('window;') is self.window)
        self.assert_(self.cx.eval_script('window.arg;') is self.window.arg)
        self.assert_(self.cx.eval_script('arg;') is self.window.arg)
        self.cx.eval_script('foo(12);')
        self.assert_(self.cx.get_global('arg') == 12)
        self.cx.eval_script('var spam = 13;')
        self.assert_(self.cx.get_global('spam') == 13)
        self.cx.eval_script('window.arg = 14;')
        self.assert_(self.cx.eval_script('window.arg;') == 14)

class test_define_function(TestCase):
    def setUp(self):
        rt = Runtime()
        self.cx = rt.new_context()
    
    def test_define_function(self):
        resp = self.cx.eval_script("function(val) {return val * 2;}")
        self.assertEqual(resp(2), 4)

    def test_with_dict(self):
        resp = self.cx.eval_script("function(doc) {if(doc.data) return doc.data;}")
        self.assertEqual(resp({"data": 2}), 2)
        self.assertEqual(resp({}), None)

if __name__ == "__main__":
    unittest.main()
