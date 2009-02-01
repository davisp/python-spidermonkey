import unittest
import spidermonkey

class ToPythonTest(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context()

    def test_primitive_types(self):
        self.assertEqual(self.cx.execute(u"42;"), 42)
        self.assertEqual(self.cx.execute(u"42.5;"), 42.5)
        self.assertEqual(self.cx.execute(u'"spam";'), u"spam")
        self.assertEqual(self.cx.execute(u"undefined;"), None)
        self.assertEqual(self.cx.execute(u"null;"), None)
        self.assertEqual(self.cx.execute(u"true;"), True)
        self.assertEqual(self.cx.execute(u"false;"), False)
        #self.assert_(self.cx.execute("Infinity;") is XXX)
        #self.assert_(self.cx.execute("NaN;") is XXX)

    def test_container_types(self):
        self.assertEqual(self.cx.execute(u"[1,2,3];"), [1,2,3])
        self.assertEqual(self.cx.execute(
            u'var d = {0: 0, "a": 1, 2: "b", "c": "d", "blah": 2.5}; d;'),
                     {0: 0, u"a": 1, 2: u"b", u"c": u"d", u"blah": 2.5})
        self.assertEqual(self.cx.execute(
            u'["foo", 2, {"bar": 2.3, "spam": [1,2,3]}];'),
                     [u"foo", 2, {u"bar": 2.3, u"spam": [1,2,3]}])

if __name__ == "__main__":
    unittest.main()
