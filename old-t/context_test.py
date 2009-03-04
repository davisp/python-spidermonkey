import unittest
import spidermonkey

class ContextTest(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context()

    def test_scope(self):
        self.cx.execute(u"var x = 42;")
        self.assertEqual(self.cx.execute(u"x;"), 42)
        
if __name__ == "__main__":
    unittest.main()