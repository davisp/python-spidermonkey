import unittest
import spidermonkey

class ContextTest(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context()

    def test_scope(self):
        self.cx.execute("var x = 42;")
        self.assertEqual(self.cx.execute("x;"), 42)