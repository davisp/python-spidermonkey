import unittest
import spidermonkey


class UnicodeTest(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        
        self.cx = rt.create_context()
    
    def roundtrip_test(self):
        self.assertEqual(u"foo", spidermonkey.test_utf_16_round_trip(self.cx, u"foo"))
        self.assertEqual(u"f\u00E9o\u00FCo", spidermonkey.test_utf_16_round_trip(self.cx, u"f\u00E9o\u00FCo"))
        self.assertRaises(TypeError, spidermonkey.test_utf_16_round_trip, self.cx, "this is not unicode")

    def execute_test(self):
        self.assertEqual(u"\u0042", self.cx.execute(u'"\u0042";'))
        # Check non-unicode entry
        self.assertRaises(TypeError, self.cx.execute, '"\u0042";')
    
    def bind(self):
        self.cx.bind(u"\u0042", 1)
        self.assertEqual(1, self.cx.execute(u'"\u0042";'))
        # Non-unicode
        self.assertRaises(TypeError, self.cx.bind, '"\u0042";', 1)
    
    def unbind(self):
        self.cx.bind(u"\u0042", 1)
        self.assertEqual(1, self.cx.execute(u'"\u0042";'))
        self.assertEqual(1, self.cx.unbind(u"\u0042"))
        # Non-unicode
        self.assertRaises(TypeError, self.cx.bind, '"\u0042";', 1)
        self.assertRaises(TypeError, self.cx.execute, '"\u0042";')
        self.assertRaises(TypeError, self.cx.unbind, '"\u0042";')

if __name__ == "__main__":
    unittest.main()
