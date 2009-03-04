import unittest
import spidermonkey

class DefineFunctionTest(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context()
    
    def test_define_function(self):
        resp = self.cx.execute(u"function(val) {return val * 2;}")
        self.assertEqual(resp(2), 4)

    def test_with_dict(self):
        resp = self.cx.execute(u"function(doc) {if(doc.data) return doc.data;}")
        self.assertEqual(resp({u"data": 2}), 2)
        self.assertEqual(resp({}), None)
    
    def test_throw(self):
        resp = self.cx.execute(u"function(doc) {throw(\"error\");}")
        self.assertRaises(spidermonkey.JSError, resp)
        
if __name__ == "__main__":
    unittest.main()