import unittest
import spidermonkey

class DefineFunctionTest(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context()
    
    def test_define_function(self):
        resp = self.cx.execute("function(val) {return val * 2;}")
        self.assertEqual(resp(2), 4)

    def test_with_dict(self):
        resp = self.cx.execute("function(doc) {if(doc.data) return doc.data;}")
        self.assertEqual(resp({"data": 2}), 2)
        self.assertEqual(resp({}), None)
    
    def test_throw(self):
        resp = self.cx.execute("function(doc) {throw(\"error\");}")
        self.assertRaises(spidermonkey.JSError, resp)