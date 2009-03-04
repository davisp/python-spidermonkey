import unittest
import spidermonkey

class BindTest(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context()

    def test_error_thrown(self):
        self.assertRaises(spidermonkey.JSError, self.cx.execute, u'throw("foo")')
        
if __name__ == "__main__":
    unittest.main()