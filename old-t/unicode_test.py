import unittest
import spidermonkey


class UnicodeTest(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context()
    
    def test_roundtrip(self):
        self.assertEqual(u"foo", spidermonkey.test_utf_16_round_trip(self.cx, u"foo"))
        self.assertEqual(u"f\u00E9o\u00FCo", spidermonkey.test_utf_16_round_trip(self.cx, u"f\u00E9o\u00FCo"))
        self.assertRaises(UnicodeError, spidermonkey.test_utf_16_round_trip, self.cx, "this is not unicode")

    def test_execute(self):
        self.assertEqual(u"\u0042", self.cx.execute(u'"\u0042";'))
        # Check non-unicode entry
        self.assertRaises(UnicodeError, self.cx.execute, '"\u0042";')
    
    def test_bind(self):
        self.cx.bind(u"\u00FC", 1)
        self.assertEqual(1, self.cx.execute(u'\u00FC;'))
        # Non-unicode
        self.assertRaises(UnicodeError, self.cx.bind, '\u0042;', 1)
    
    def test_bind_function(self):
        # Make sure we throw an error with non-unicode return.
        def error():
            return "foo"
        self.cx.bind(u"error", error)
        self.assertRaises(spidermonkey.JSError, self.cx.execute, u"error();")
    
    def test_unbind(self):
        self.cx.bind(u"\u00E9", 1)
        self.assertEqual(1, self.cx.execute(u'\u00E9;'))
        self.assertEqual(1, self.cx.unbind(u"\u00E9"))
        # Non-unicode
        self.assertRaises(UnicodeError, self.cx.bind, '\u00E9;', 1)
        self.assertRaises(UnicodeError, self.cx.execute, '\u00E9;')
        self.assertRaises(UnicodeError, self.cx.unbind, '\u00E9;')

if __name__ == '__main__':
    unittest.main()
