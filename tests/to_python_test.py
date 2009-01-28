import unittest
import spidermonkey

class ToPythonTest(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context()
    def test_primitive_types(self):
        self.assertEqual(self.cx.execute("42;"), 42)
        self.assertEqual(self.cx.execute("42.5;"), 42.5)
        self.assertEqual(self.cx.execute('"spam";'), "spam")
        self.assertEqual(self.cx.execute("undefined;"), None)
        self.assertEqual(self.cx.execute("null;"), None)
        self.assertEqual(self.cx.execute("true;"), True)
        self.assertEqual(self.cx.execute("false;"), False)
        #self.assert_(self.cx.execute("Infinity;") is XXX)
        #self.assert_(self.cx.execute("NaN;") is XXX)

    def test_container_types(self):
        self.assertEqual(self.cx.execute("[1,2,3];"), [1,2,3])
        self.assertEqual(self.cx.execute(
            'var d = {0: 0, "a": 1, 2: "b", "c": "d", "blah": 2.5}; d;'),
                     {0: 0, "a": 1, 2: "b", "c": "d", "blah": 2.5})
        self.assertEqual(self.cx.execute(
            '["foo", 2, {"bar": 2.3, "spam": [1,2,3]}];'),
                     ["foo", 2, {"bar": 2.3, "spam": [1,2,3]}])